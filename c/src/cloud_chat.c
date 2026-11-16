/* =============================================================================
 * cloud_chat.c —— 移远云端对话会话实现
 *
 * ## 关键实现细节（都是实测踩坑得出的，改动前务必读）
 *
 * 1. **必须 Opus，不能 PCM16**
 *    用 PCM16 发音频，服务端收到后会立刻 close（实测 0.7s 内）。
 *
 * 2. **必须 protocol=LOCAL**
 *    REALTIME 模式下服务端行为不同，会话建不起来。
 *
 * 3. **audio_size_per_pkg 必须 4800**
 *    参考项目实测注释：3200(100ms) → 下发仅 0.84×实时会卡；
 *    6400(200ms) → 1.24×实时缓冲堆积、播放变速；4800(150ms) ≈1.05× 最优。
 *
 * 4. **上行帧必须是 10ms = 160 采样**
 *    Opus 编码器固定帧长，传别的长度 opus_encode 会失败。
 *
 * 5. **SDK 的两个协议字段名 bug**（在 qthsdk 里已 patch，见 patch 说明）
 *    - 事件类型：SDK 读 "eventType"，服务端发 "type"
 *    - 会话字段：SDK 读 "sid"/"msgId"/"mode"，服务端发
 *      "session_id"/"event_id"，mode 在嵌套 session 对象里
 *    不 patch 的话：WebSocket 连上了、服务端也有数据回来，但**回调一次都不触发**。
 * ========================================================================== */

#include "cloud_chat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>

/* ---- qthsdk ---- */
#include "qhal_types.h"
#include "qth_core.h"
#include "qth_config.h"
#include "qth_aiChat.h"
#include "qth_dev.h"
#include "qth_dmpConn.h"
#include "quos_log.h"   /* Quos_logSetUsrLevel：抑制 SDK 每帧刷屏 */

/* ---- Opus ----
 * 板上只装了 libopus.so.0（运行库），**没有开发头文件**，
 * 所以这里直接声明用到的几个符号，避免额外依赖 opus dev 包。*/
typedef struct OpusEncoder OpusEncoder;
typedef struct OpusDecoder OpusDecoder;
#define OPUS_OK                    0
#define OPUS_APPLICATION_VOIP      2048
#define OPUS_SET_BITRATE(x)        (4002 | ((x) << 5))
#define OPUS_SET_SIGNAL(x)         (4024 | ((x) << 5))
#define OPUS_SIGNAL_VOICE          3001
extern OpusEncoder *opus_encoder_create(int Fs, int channels, int application, int *error);
extern int   opus_encoder_ctl(OpusEncoder *st, int request, ...);
extern int   opus_encode(OpusEncoder *st, const short *pcm, int frame_size,
                         unsigned char *data, int max_data_bytes);
extern void  opus_encoder_destroy(OpusEncoder *st);
extern OpusDecoder *opus_decoder_create(int Fs, int channels, int *error);
extern int   opus_decode(OpusDecoder *st, const unsigned char *data,
                         int len, short *pcm, int frame_size, int decode_fec);
extern void  opus_decoder_destroy(OpusDecoder *st);

/* =============================================================================
 * 常量
 * ========================================================================== */
#define CC_PK   "p11HGD"
#define CC_PS   "Y3lXengvQVBxWTc0"
#define CC_DK   "869598079850031"
#define CC_URL  "wss://uat-ai-chat.quectel.com.cn/ws/v1/chat"

#define CC_SR              16000
#define CC_FRAME_SAMPLES   160       /* 10ms @16k */
#define CC_FRAME_BYTES     (CC_FRAME_SAMPLES * 2)
#define CC_OPUS_MAX        1275
#define CC_BITRATE         24000
#define CC_PKG_SIZE        4800      /* 150ms，实测最优吞吐 */
#define CC_TEXT_MAX        1024

/* =============================================================================
 * 会话结构
 * ========================================================================== */
struct cc_s {
    pthread_mutex_t lock;

    Qth_aiChatHandle_t handle;
    OpusEncoder       *enc;
    OpusDecoder       *dec;

    cc_state_t         state;
    volatile int       ready;
    volatile int       stopped;

    /* 上行帧拼装：上层给的块不一定是 160 采样整数倍 */
    int16_t            tx_acc[CC_FRAME_SAMPLES];
    int                tx_fill;

    /* 最近一次问答（供 UI）*/
    char               question[CC_TEXT_MAX];
    char               reply[CC_TEXT_MAX];

    cc_stats_t         stats;

    /* 下行音频速率诊断：如果云端下发比实时慢，播放就会欠载。
     * 记录「首块音频到最后一块的墙钟时间」和「对应音频时长」对比。*/
    double             rx_first_at;
    double             rx_last_at;
    unsigned long      rx_samples;
    cc_callbacks_t     cb;

    /* 自动重连
     *
     * 服务端有 10 分钟空闲超时（600000ms），超时会主动断开 WebSocket。
     * 断开后 SDK 不会自己重连，程序就永久停在"待机"，说话也没反应——
     * 展会现场没人会去手动重启，所以必须自己加自愈。
     *
     * 做法：回调里只置标志位（回调在 SDK 线程里跑，绝不能在里面做重连
     * 这种阻塞操作），由一条独立的重连线程轮询标志并执行重建。*/
    volatile int       need_reconnect;
    pthread_t          reconn_thread;
    volatile int       reconn_running;

    /* 保存一份配置，重连时要重新 Init */
    Qth_aiChatConfig_t cfg;
};

/*: 全局单例指针（SDK 回调只给 handle + user_context，这里用 cc 作 context）*/
static cc_t *g_cc = NULL;

/* =============================================================================
 * 小工具
 * ========================================================================== */
static void set_state(cc_t *cc, cc_state_t st)
{
    if (cc->state == st) return;
    cc->state = st;
    if (cc->cb.on_state) cc->cb.on_state(st, cc->cb.user);
}

static void emit_error(cc_t *cc, int code, const char *msg)
{
    fprintf(stderr, "[CC] 错误 code=%d msg=%s\n", code, msg ? msg : "");
    if (cc->cb.on_error) cc->cb.on_error(code, msg, cc->cb.user);
}

/* 上行：把 PCM 拼成 160 采样帧后 Opus 编码上云 */
static void tx_push(cc_t *cc, const int16_t *pcm, size_t n)
{
    size_t i = 0;
    while (i < n) {
        int space = CC_FRAME_SAMPLES - cc->tx_fill;
        int take = (int)((n - i) < (size_t)space ? (n - i) : (size_t)space);
        memcpy(cc->tx_acc + cc->tx_fill, pcm + i, (size_t)take * sizeof(int16_t));
        cc->tx_fill += take;
        i += (size_t)take;

        if (cc->tx_fill < CC_FRAME_SAMPLES) break;

        /* 一帧满了 → 编码发送 */
        unsigned char pkt[CC_OPUS_MAX];
        int len = opus_encode(cc->enc, cc->tx_acc, CC_FRAME_SAMPLES,
                              pkt, (int)sizeof(pkt));
        if (len > 0 && cc->handle) {
            Qth_aiChatSendAudio(cc->handle, pkt, (quint32_t)len, 3000);
            cc->stats.frames_tx++;
            cc->stats.audio_tx_bytes += (unsigned long)len;
        }
        cc->tx_fill = 0;
    }
}

/* =============================================================================
 * SDK 事件回调
 * ========================================================================== */
static void on_sdk_event(Qth_aiChatHandle_t h, Qth_aiChatEventId_e id,
                         void *data, void *ctx)
{
    cc_t *cc = (cc_t *)ctx;
    (void)h;
    if (!cc || cc->stopped) return;

    switch (id) {
    case QTH_AI_CHAT_EVENT_CONNECTED:
        fprintf(stderr, "[CC] WebSocket 已连接\n");
        break;

    case QTH_AI_CHAT_EVENT_DISCONNECTED:
        fprintf(stderr, "[CC] WebSocket 断开，准备重连\n");
        cc->ready = 0;
        set_state(cc, CC_STATE_CONNECTING);
        /* 只置标志，实际重连交给独立线程做
         * （这里是 SDK 的回调线程，阻塞在这里会拖死整个收发循环）*/
        cc->need_reconnect = 1;
        break;

    case QTH_AI_CHAT_EVENT_SESSION_CREATED: {
        Qth_aiChatSessionCreatedData_t *d = data;
        fprintf(stderr, "[CC] 会话就绪 sid=%.24s in=%d out=%d\n",
                d ? (char *)d->sid : "?", d ? (int)d->input_audio_format : -1,
                d ? (int)d->output_audio_format : -1);
        cc->ready = 1;
        set_state(cc, CC_STATE_READY);
        break;
    }

    case QTH_AI_CHAT_EVENT_VAD_STARTED:
        fprintf(stderr, "[CC] 用户开始说话\n");
        pthread_mutex_lock(&cc->lock);
        cc->question[0] = 0;
        pthread_mutex_unlock(&cc->lock);
        set_state(cc, CC_STATE_USER_SPEAK);
        break;

    case QTH_AI_CHAT_EVENT_VAD_STOPPED:
        fprintf(stderr, "[CC] 用户停止说话\n");
        break;

    case QTH_AI_CHAT_EVENT_TRANSCRIPT_DELTA:
    case QTH_AI_CHAT_EVENT_TRANSCRIPT_COMPLETED: {
        Qth_aiChatMessageData_t *d = data;
        if (!d || !d->content || !d->content_len) break;
        int done = (id == QTH_AI_CHAT_EVENT_TRANSCRIPT_COMPLETED);
        cc->stats.asr_deltas++;
        if (cc->cb.on_asr) cc->cb.on_asr((const char *)d->content, !done,
                                         cc->cb.user);
        if (done) {
            pthread_mutex_lock(&cc->lock);
            size_t n = d->content_len < sizeof(cc->question) - 1
                       ? d->content_len : sizeof(cc->question) - 1;
            memcpy(cc->question, d->content, n);
            cc->question[n] = 0;
            pthread_mutex_unlock(&cc->lock);
            set_state(cc, CC_STATE_THINKING);
        }
        break;
    }

    case QTH_AI_CHAT_EVENT_MESSAGE_DELTA:
    case QTH_AI_CHAT_EVENT_MESSAGE_COMPLETED: {
        Qth_aiChatMessageData_t *d = data;
        if (!d || !d->content || !d->content_len) break;
        int done = (id == QTH_AI_CHAT_EVENT_MESSAGE_COMPLETED);
        cc->stats.llm_deltas++;
        if (cc->cb.on_reply) cc->cb.on_reply((const char *)d->content, !done,
                                            cc->cb.user);
        if (done) {
            pthread_mutex_lock(&cc->lock);
            size_t n = d->content_len < sizeof(cc->reply) - 1
                       ? d->content_len : sizeof(cc->reply) - 1;
            memcpy(cc->reply, d->content, n);
            cc->reply[n] = 0;
            pthread_mutex_unlock(&cc->lock);
        }
        break;
    }

    case QTH_AI_CHAT_EVENT_AUDIO_OUTPUT_STARTED: {
        struct timespec ts;
        clock_gettime(CLOCK_MONOTONIC, &ts);
        cc->rx_first_at = (double)ts.tv_sec + ts.tv_nsec / 1e9;
        cc->rx_last_at = 0.0;
        cc->rx_samples = 0;
        set_state(cc, CC_STATE_SPEAKING);
        break;
    }

    case QTH_AI_CHAT_EVENT_AUDIO_OUTPUT_DATA: {
        Qth_aiChatAudioChunkData_t *d = data;
        if (!d || !d->data || !d->len) break;
        cc->stats.audio_rx_bytes += d->len;

        /* 下行也是 Opus，先解码成 PCM 再交给上层 */
        short pcm[CC_SR / 1000 * 120];        /* 最多 120ms */
        int n = opus_decode(cc->dec, (const unsigned char *)d->data,
                            (int)d->len, pcm, (int)(sizeof(pcm) / sizeof(short)), 0);
        if (n > 0) {
            struct timespec ts;
            clock_gettime(CLOCK_MONOTONIC, &ts);
            cc->rx_last_at = (double)ts.tv_sec + ts.tv_nsec / 1e9;
            cc->rx_samples += (unsigned long)n;
            if (cc->cb.on_audio) cc->cb.on_audio(pcm, (size_t)n, cc->cb.user);
        }
        break;
    }

    case QTH_AI_CHAT_EVENT_AUDIO_OUTPUT_COMPLETED: {
        /* 下发速率 = 音频时长 / 墙钟时长。小于 1 说明云端供不上，必然欠载。*/
        if (cc->rx_last_at > cc->rx_first_at && cc->rx_samples > 0) {
            double wall = cc->rx_last_at - cc->rx_first_at;
            double audio = (double)cc->rx_samples / 16000.0;
            fprintf(stdout, "[CC] TTS 结束 音频%.2fs 耗时%.2fs 速率%.2fx%s\n",
                    audio, wall, audio / (wall > 0.001 ? wall : 0.001),
                    (audio / (wall > 0.001 ? wall : 0.001)) < 0.95
                        ? " ⚠供不上" : "");
        } else {
            fprintf(stderr, "[CC] TTS 结束\n");
        }
        set_state(cc, cc->ready ? CC_STATE_READY : CC_STATE_IDLE);
        break;
    }

    case QTH_AI_CHAT_EVENT_CONVERSATION_CANCELLED:
        fprintf(stderr, "[CC] 对话被打断\n");
        break;

    case QTH_AI_CHAT_EVENT_ERROR: {
        Qth_aiChatErrorData_t *d = data;
        char msg[288];
        snprintf(msg, sizeof(msg), "code=%u http=%u %s",
                 d ? d->code : 0, d ? d->http_status_code : 0,
                 d ? (char *)d->msg : "");
        emit_error(cc, d ? (int)d->code : -1, msg);
        set_state(cc, CC_STATE_ERROR);
        break;
    }

    default:
        break;
    }
}

/* =============================================================================
 * 自动重连
 *
 * 服务端空闲 10 分钟（600000ms）会主动断开 WebSocket，SDK 不会自动恢复。
 * 展会现场程序要能连续跑一整天，所以必须自己重连。
 *
 * 设计要点：
 *   - 回调里只置 need_reconnect 标志，绝不在回调线程里做重连（会阻塞收发）；
 *   - 本线程负责真正重建，带退避（2s 起，失败翻倍，最多 15s）；
 *   - 重连期间 state 是 CONNECTING，界面会显示"连接中"而不是骗人的"待机"。
 * ========================================================================== */
static void *reconnect_thread(void *arg)
{
    cc_t *cc = (cc_t *)arg;
    int backoff_ms = 2000;

    fprintf(stderr, "[CC] 重连线程启动\n");

    while (cc->reconn_running && !cc->stopped) {
        if (!cc->need_reconnect) {
            usleep(500 * 1000);
            continue;
        }

        /* 等 SDK 把断开这件事处理完，别立刻又连 */
        usleep(800 * 1000);
        cc->need_reconnect = 0;
        if (cc->stopped) break;

        fprintf(stderr, "[CC] 开始重连（退避 %dms）…\n", backoff_ms);

        /* 销毁旧的句柄。SDK 要求先 Disconnect 再 Destroy，
         * 但断线状态下 Disconnect 可能返回错误，忽略即可。*/
        if (cc->handle) {
            Qth_aiChatDisconnect(cc->handle);
            Qth_aiChatDestroy(cc->handle);
            cc->handle = NULL;
        }
        cc->ready = 0;

        /* ⚠️ Qth_aiChatInit 只能调一次（内部会建 socket 线程）。
         * 重连时只需要重新 Connect，不要重复 Init。*/
        if (!cc->handle) {
            cc->handle = Qth_aiChatInit(&cc->cfg);
            if (!cc->handle) {
                fprintf(stderr, "[CC] ✗ 重新 Init 失败，%dms 后重试\n", backoff_ms);
                cc->need_reconnect = 1;
                usleep((useconds_t)backoff_ms * 1000);
                backoff_ms *= 2;
                if (backoff_ms > 15000) backoff_ms = 15000;
                continue;
            }
        }

        int ok = 0;
        if (cc->handle && Qth_aiChatConnect(cc->handle) == OPRT_OK) {
            /* 等 session.created，最多 15s */
            for (int i = 0; i < 150 && cc->reconn_running && !cc->stopped; i++) {
                if (cc->ready) { ok = 1; break; }
                usleep(100 * 1000);
            }
        }

        if (ok) {
            fprintf(stderr, "[CC] ✓ 重连成功\n");
            backoff_ms = 2000;                 /* 成功就复位退避 */
            set_state(cc, CC_STATE_READY);
        } else {
            fprintf(stderr, "[CC] ✗ 重连失败，%dms 后重试\n", backoff_ms);
            if (cc->handle) {
                Qth_aiChatDisconnect(cc->handle);
                Qth_aiChatDestroy(cc->handle);
                cc->handle = NULL;
            }
            cc->need_reconnect = 1;            /* 继续重试 */
            usleep((useconds_t)backoff_ms * 1000);
            backoff_ms *= 2;
            if (backoff_ms > 15000) backoff_ms = 15000;
        }
    }

    fprintf(stderr, "[CC] 重连线程退出\n");
    return NULL;
}

/* =============================================================================
 * 建链
 * ========================================================================== */
cc_t *cc_start(const cc_callbacks_t *cb)
{
    cc_t *cc = (cc_t *)calloc(1, sizeof(*cc));
    if (!cc) return NULL;

    pthread_mutex_init(&cc->lock, NULL);
    if (cb) cc->cb = *cb;
    cc->state = CC_STATE_CONNECTING;
    cc->tx_fill = 0;
    g_cc = cc;

    /* 抑制 SDK 的 DBG/INFO 刷屏。
     * WebSocket 每个心跳、每帧音频都会打一条 DBG，一秒几十行，
     * 不关的话应用自己的日志会被完全淹没。只保留 WARN 及以上。 */
    Quos_logSetUsrLevel(LL_WARN);

    /* ---- 1. SDK 内核 ---- */
    oprt_ret r = Qth_coreInit();
    if (r != OPRT_OK) {
        emit_error(cc, (int)r, "Qth_coreInit 失败");
        goto fail;
    }

    /* ---- 2. 凭据 ---- */
    Qth_configSetProductInfo(CC_PK, CC_PS);
    Qth_configSetDeviceInfo(CC_DK, NULL);   /* ds 走 DMP 动态注册 */
    Qth_configSetServer(QTH_DEFAULT_SERVERURL_TLS, QTH_DEFAULT_SERVERIP);

    /* ---- 3. 设备服务 + DMP 注册（拿 device_secret）---- */
    r = Qth_devStart();
    if (r != OPRT_OK) {
        emit_error(cc, (int)r, "Qth_devStart 失败");
        goto fail;
    }
    Qth_dmpConnect();

    fprintf(stderr, "[CC] 等待 DMP 注册…\n");
    for (int i = 0; i < 100 && !cc->stopped; i++) {   /* 最多 10s */
        if (Qth_dmpConnGetStatus()) break;
        usleep(100 * 1000);
    }
    if (!Qth_dmpConnGetStatus()) {
        emit_error(cc, -100, "DMP 注册超时（检查网络 / 凭据）");
        goto fail;
    }
    fprintf(stderr, "[CC] DMP 注册成功\n");

    /* ---- 4. Opus 编解码器 ---- */
    int oerr = 0;
    cc->enc = opus_encoder_create(CC_SR, 1, OPUS_APPLICATION_VOIP, &oerr);
    if (!cc->enc || oerr != OPUS_OK) {
        emit_error(cc, oerr, "opus_encoder_create 失败");
        goto fail;
    }
    opus_encoder_ctl(cc->enc, OPUS_SET_BITRATE(CC_BITRATE));
    opus_encoder_ctl(cc->enc, OPUS_SET_SIGNAL(OPUS_SIGNAL_VOICE));

    cc->dec = opus_decoder_create(CC_SR, 1, &oerr);
    if (!cc->dec || oerr != OPUS_OK) {
        emit_error(cc, oerr, "opus_decoder_create 失败");
        goto fail;
    }

    /* ---- 5. AI Chat 会话 ---- */
    Qth_aiChatConfig_t cfg;
    memset(&cfg, 0, sizeof(cfg));
    cfg.url                     = (const quint8_t *)CC_URL;
    cfg.mode                    = QTH_AI_CHAT_MODE_STT_LLM_TTS;
    cfg.protocol                = QTH_AI_CHAT_PROTOCOL_LOCAL;   /* 必须 LOCAL */
    cfg.input_audio_format      = QTH_AI_CHAT_AUDIO_FMT_OPUS;   /* 必须 Opus */
    cfg.output_audio_format     = QTH_AI_CHAT_AUDIO_FMT_OPUS;
    cfg.opus_output_use_cbr     = TRUE;
    cfg.opus_output_frame_size_ms = 60;
    cfg.audio_push_interval     = 100;
    cfg.audio_size_per_pkg      = CC_PKG_SIZE;
    cfg.ws_buffer_size          = 30 * 1024;
    cfg.debug                   = 0;      /* 生产环境关掉，否则日志刷屏 */
    cfg.event_handler           = on_sdk_event;
    cfg.user_context            = cc;

    /* 存一份到结构体，重连线程要用它重新 Init。
     * ⚠️ cfg.url 指向的是常量字符串，可以安全保留。*/
    cc->cfg = cfg;

    cc->handle = Qth_aiChatInit(&cfg);
    if (!cc->handle) {
        emit_error(cc, -101, "Qth_aiChatInit 失败");
        goto fail;
    }

    r = Qth_aiChatConnect(cc->handle);
    if (r != OPRT_OK) {
        emit_error(cc, (int)r, "Qth_aiChatConnect 失败");
        goto fail;
    }

    /* 等 session.created */
    fprintf(stderr, "[CC] 等待会话建立…\n");
    for (int i = 0; i < 150 && !cc->stopped; i++) {   /* 最多 15s */
        if (cc->ready) break;
        usleep(100 * 1000);
    }
    if (!cc->ready) {
        emit_error(cc, -102, "会话建立超时");
        goto fail;
    }

    fprintf(stderr, "[CC] ✓ 云端会话就绪\n");

    /* 起自动重连线程：服务端 10 分钟空闲会主动断开，
     * 没有这个线程程序会永久卡在"待机"。*/
    cc->reconn_running = 1;
    if (pthread_create(&cc->reconn_thread, NULL, reconnect_thread, cc) != 0) {
        fprintf(stderr, "[CC] 重连线程启动失败（断开后不会自愈）\n");
        cc->reconn_running = 0;
    }
    return cc;

fail:
    cc_stop(cc);
    return NULL;
}

void cc_stop(cc_t *cc)
{
    if (!cc) return;
    cc->stopped = 1;

    /* 先停重连线程，否则它可能正在重建句柄，和下面的清理打架 */
    if (cc->reconn_running) {
        cc->reconn_running = 0;
        pthread_join(cc->reconn_thread, NULL);
    }

    if (cc->handle) {
        Qth_aiChatDisconnect(cc->handle);
        Qth_aiChatDestroy(cc->handle);
        cc->handle = NULL;
    }
    if (cc->enc) { opus_encoder_destroy(cc->enc); cc->enc = NULL; }
    if (cc->dec) { opus_decoder_destroy(cc->dec); cc->dec = NULL; }

    pthread_mutex_destroy(&cc->lock);
    if (g_cc == cc) g_cc = NULL;
    free(cc);
}

/* =============================================================================
 * 对外音频 / 状态接口
 * ========================================================================== */
void cc_feed_pcm(cc_t *cc, const int16_t *pcm, size_t samples)
{
    if (!cc || cc->stopped || !pcm || samples == 0) return;
    /* 未就绪时丢弃，避免缓冲无意义增长 */
    if (!cc->ready || !cc->enc) return;
    pthread_mutex_lock(&cc->lock);
    tx_push(cc, pcm, samples);
    pthread_mutex_unlock(&cc->lock);
}

void cc_cancel(cc_t *cc)
{
    if (!cc || !cc->handle) return;
    Qth_aiChatCancelConversation(cc->handle);
}

cc_state_t cc_state(const cc_t *cc)
{
    return cc ? cc->state : CC_STATE_ERROR;
}

int cc_is_ready(const cc_t *cc)
{
    return cc ? cc->ready : 0;
}

const char *cc_last_question(const cc_t *cc)
{
    return cc ? cc->question : "";
}

const char *cc_last_reply(const cc_t *cc)
{
    return cc ? cc->reply : "";
}

void cc_get_stats(const cc_t *cc, cc_stats_t *out)
{
    if (!cc || !out) return;
    *out = cc->stats;
}
