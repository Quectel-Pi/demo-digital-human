/* =============================================================================
 * main.c —— DEMO-030 语音对话主程序（纯 C）
 *
 * 链路：
 *
 *   麦克风 ──ALSA采集──┐
 *                      ├─► cc_feed_pcm() ─► Opus编码 ─► WebSocket ─► 云端
 *   喇叭 ◄──ALSA播放◄──┘                                          │
 *                                                                 │
 *        ┌──────────────────────────────────────────────────────────┘
 *        │  云端：VAD切句 → ASR(volcengine流式) → LLM → TTS(流式)
 *        ▼
 *   on_asr / on_reply / on_audio 回调
 *
 * 全链路延迟构成（云端一体化，本地只做音频收发）：
 *   采集延迟 ~20ms + 上行网络 + 云端ASR/LLM/TTS + 下行网络 + 播放缓冲 ~200ms
 *
 * 编译见 Makefile。
 * ========================================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>

#include "audio_io.h"
#include "cloud_chat.h"
#include "avatar.h"

/* =============================================================================
 * 配置
 * ========================================================================== */
#define CAP_DEVICE   "plughw:0,0"
#define PLAY_DEVICE  "plughw:0,0"
#define SAMPLE_RATE  16000
#define CAP_BLOCK_MS 20

/* =============================================================================
 * 全局状态
 * ========================================================================== */
static volatile int g_run = 1;
static audio_play_t g_play;
static cc_t        *g_cc = NULL;
static avatar_t    *g_av = NULL;

/* 界面上显示的回答全文。
 * 不能直接用 cb_reply 里的 acc —— 那个缓冲会随断句不断被 memmove 吃掉，
 * 只保留"还没打印的尾巴"。界面要的是整段回答，所以单独攒一份。*/
static char   g_disp[1200];
static size_t g_disp_len = 0;

/* 把播放位置喂给数字人模块（避免 avatar 直接依赖 audio_io）*/
static unsigned long long play_pos_cb(void *user)
{
    (void)user;
    return audio_play_position(&g_play);
}

static volatile int g_asr_n = 0;
static volatile int g_reply_n = 0;
static volatile int g_audio_chunks = 0;
static volatile size_t g_audio_samples = 0;
static double g_first_audio_at = 0.0;
static double g_t0 = 0.0;

static double now_s(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + ts.tv_nsec / 1e9;
}

static void on_sig(int s) { (void)s; g_run = 0; }

/* =============================================================================
 * 云端回调
 * ========================================================================== */
static void cb_state(cc_state_t st, void *user)
{
    (void)user;
    const char *s = "?";
    switch (st) {
    case CC_STATE_IDLE:       s = "空闲";     break;
    case CC_STATE_CONNECTING: s = "连接中";   break;
    case CC_STATE_READY:      s = "就绪";     break;
    case CC_STATE_USER_SPEAK: s = "听你说";   break;
    case CC_STATE_THINKING:   s = "思考中";   break;
    case CC_STATE_SPEAKING:   s = "回答中";   break;
    case CC_STATE_ERROR:      s = "错误";     break;
    }
    printf("\n[状态] %s\n", s);
    fflush(stdout);

    /* 同步数字人状态机（表情 + 口型开关）*/
    if (g_av) {
        switch (st) {
        case CC_STATE_USER_SPEAK: av_set_state(g_av, AV_STATE_LISTEN); break;
        case CC_STATE_THINKING:   av_set_state(g_av, AV_STATE_THINK);  break;
        case CC_STATE_SPEAKING:   av_set_state(g_av, AV_STATE_SPEAK);  break;
        case CC_STATE_READY:
        case CC_STATE_IDLE:       av_set_state(g_av, AV_STATE_IDLE);   break;
        default: break;
        }
    }

    /* 延迟基准：必须取「用户说完」的时刻，不是「开始说话」！
     * 取开始说话会把用户自己讲话的时长算进去（2~4 秒），
     * 实测会算出 3.37s 这种虚高数字，毫无意义。*/
    if (st == CC_STATE_THINKING) {
        g_t0 = now_s();
        g_first_audio_at = 0.0;
    }
}

static void cb_asr(const char *text, int is_delta, void *user)
{
    (void)user;
    g_asr_n++;
    if (is_delta) return;      /* 增量刷屏太吵，只在最终结果时打印 */
    printf("\n[你] %s\n", text ? text : "");
    fflush(stdout);

    if (g_av && text) {
        /* 新一轮问题进来，清掉上一轮的回答，界面上不留旧内容 */
        g_disp_len = 0;
        g_disp[0] = 0;
        av_set_text(g_av, text, "");
    }
}

/* =============================================================================
 * LLM 回复的流式清洗 + 打印
 *
 * 云端模型会在回答最前面加一个情绪标注（"（疑问）"、"(高兴)" 等），供前端
 * 切换表情用。但它在纯文本回答里很难看，显示前必须剥掉。
 *
 * ⚠️ 这里有两个坑，都是实测踩出来的：
 *
 * 1. **标注会被拆到多个增量包里**，实测收到的序列是 "（" → "\n" → "疑问）"
 *    → "\n" → 正文。所以不能对每个增量包单独做剥离，必须把整段回复攒起来，
 *    等开头的括号组收完整了再判断。
 *
 * 2. **标注内部带换行**。如果把 '\n' 当成断句符（最初就是这么写的），
 *    就会在 "（" 后面立刻打印，把标注劈成 "[答] （" 和 "[答] 疑问）" 两行。
 *    所以断句只认 。！？，换行一律当空白处理。
 * ========================================================================== */

/* 打印时把内嵌换行替换成空格，避免一条回答断成好几行 */
static void print_clean(const char *text, size_t len)
{
    char line[2048];
    size_t j = 0;
    for (size_t i = 0; i < len && j < sizeof(line) - 1; i++) {
        char c = text[i];
        line[j++] = (c == '\n' || c == '\r' || c == '\t') ? ' ' : c;
    }
    /* 去掉首尾空白 */
    while (j > 0 && line[j - 1] == ' ') j--;
    size_t k = 0;
    while (k < j && line[k] == ' ') k++;
    if (j > k) {
        printf("[答] %.*s\n", (int)(j - k), line + k);
        fflush(stdout);
    }
}

/* 尝试剥掉开头的情绪标注。
 * 返回消耗掉的字节数；若括号组还没收完整返回 0（等下一个包）；
 * 若判定不是情绪标注（没有括号 / 内容太长）返回 -1（正常文本，别再试了）。*/
static int strip_emotion_head(const char *s, size_t len)
{
    if (len == 0) return 0;

    const char *close;
    size_t open_len;
    if ((unsigned char)s[0] == 0xEF && len >= 3
        && (unsigned char)s[1] == 0xBC && (unsigned char)s[2] == 0x88) {
        close = "\xEF\xBC\x89";     /* 全角 （ … ） */
        open_len = 3;
    } else if (s[0] == '(') {
        close = ")"; open_len = 1;
    } else if (s[0] == '[') {
        close = "]"; open_len = 1;
    } else {
        return -1;                  /* 开头不是括号，没有标注 */
    }

    /* 前 3 字节可能只收到了半个字符，数据不够，等 */
    if (len < open_len) return 0;

    size_t clen = strlen(close);
    for (size_t i = open_len; i + clen <= len; i++) {
        if (memcmp(s + i, close, clen) == 0) {
            size_t body = i - open_len;
            /* 标注内容不会太长；超过 18 字节（约 6 个汉字）说明是正文里的
             * 括注，比如"（这个问题需要先确认型号）"，不能删 */
            if (body > 18) return -1;
            size_t used = i + clen;
            while (used < len && (s[used] == ' ' || s[used] == '\n'
                                  || s[used] == '\r' || s[used] == '\t')) used++;
            return (int)used;
        }
    }

    /* 还没找到右括号：如果已经收了很多字节肯定不是短标注，放弃 */
    return (len - open_len > 18) ? -1 : 0;
}

static void cb_reply(const char *text, int is_delta, void *user)
{
    (void)user;
    g_reply_n++;
    if (!text) return;

    /* 整段回复的累积缓冲（跨回调保持）*/
    static char  acc[4096];
    static size_t alen   = 0;
    static int    head_done = 0;   /* 开头的情绪标注是否已处理完 */
    static int    saw_delta = 0;   /* 本轮是否收到过增量包 */

    if (alen == 0) head_done = 0;   /* 新的一轮回答，重新开始判断 */

    /* ⚠️ MESSAGE_COMPLETED 携带的是**整段回答的全文**，不是空串，也不是
     * 最后一片。如果本轮已经收到过增量包，那些内容早就打印出去了，再把
     * 全文塞进 acc 就会重复输出一遍（表现为每条回答打印两次）。
     * 只有一种情况要用 completed 的 content：整轮一个增量包都没收到。*/
    if (!is_delta && saw_delta) {
        if (alen > 0) print_clean(acc, alen);   /* 只补打尾巴 */
        alen = 0; acc[0] = 0;
        head_done = 0; saw_delta = 0;
        return;
    }

    size_t n = strlen(text);
    if (alen + n < sizeof(acc) - 1) {
        memcpy(acc + alen, text, n);
        alen += n;
        acc[alen] = 0;
    }

    if (is_delta) saw_delta = 1;

    /* 1. 先处理开头的情绪标注 */
    if (!head_done) {
        int used = strip_emotion_head(acc, alen);
        if (used > 0) {
            memmove(acc, acc + used, alen - (size_t)used);
            alen -= (size_t)used;
            acc[alen] = 0;
            head_done = 1;
        } else if (used < 0) {
            head_done = 1;          /* 不是标注，当正文处理 */
        } else if (is_delta) {
            return;                 /* 括号组未收完整，先不输出 */
        }
    }

    /* 2. 按句号断句输出：找最后一个 。！？，把前面整段打印出来 */
    size_t cut = 0;
    for (size_t i = 0; i + 3 <= alen; i++) {
        if ((unsigned char)acc[i] == 0xE3 || (unsigned char)acc[i] == 0xEF) {
            /* 。=E3 80 82  ！=EF BC 81  ？=EF BC 9F */
            if (memcmp(acc + i, "\xE3\x80\x82", 3) == 0) cut = i + 3;
            else if (memcmp(acc + i, "\xEF\xBC\x81", 3) == 0) cut = i + 3;
            else if (memcmp(acc + i, "\xEF\xBC\x9F", 3) == 0) cut = i + 3;
        } else if (acc[i] == '!' || acc[i] == '?') {
            cut = i + 1;
        } else if (acc[i] == '.') {
            /* 英文句点：后面跟空格才算断句，避免把 3.14 拆开 */
            if (i + 1 < alen && acc[i + 1] == ' ') cut = i + 2;
        }
    }

    if (cut > 0) {
        /* 先把这一段追加到界面文本（要在 memmove 之前取内容）*/
        if (g_av && cut < sizeof(g_disp) - g_disp_len - 1) {
            memcpy(g_disp + g_disp_len, acc, cut);
            g_disp_len += cut;
            g_disp[g_disp_len] = 0;
            av_set_text(g_av, NULL, g_disp);
        }
        print_clean(acc, cut);
        memmove(acc, acc + cut, alen - cut);
        alen -= cut;
        acc[alen] = 0;
    }

    /* 3. 收到 completed（is_delta=0）说明这轮结束了，把尾巴也打出来。
     *
     * ⚠️ 尾巴这一段也必须追加到界面文本，否则界面上会缺最后一句——
     * 因为按句号断句的循环只处理"以。！？结尾"的完整句，最后一句
     * 如果没有句号就留在 acc 里，只 print 不进 g_disp。 */
    if (!is_delta) {
        if (alen > 0) {
            if (g_av && alen < sizeof(g_disp) - g_disp_len - 1) {
                memcpy(g_disp + g_disp_len, acc, alen);
                g_disp_len += alen;
                g_disp[g_disp_len] = 0;
                av_set_text(g_av, NULL, g_disp);
            }
            print_clean(acc, alen);
        }
        alen = 0;
        acc[0] = 0;
        head_done = 0;
        saw_delta = 0;
    }
}

static void cb_audio(const int16_t *pcm, size_t samples, void *user)
{
    (void)user;
    if (!pcm || samples == 0) return;
    if (g_first_audio_at == 0.0) {
        g_first_audio_at = now_s();
        if (g_t0 > 0.0) {
            printf("\n[延迟] 说完 → 出声 %.2fs\n", g_first_audio_at - g_t0);
        }
    }
    g_audio_chunks++;
    g_audio_samples += samples;

    /* 口型必须在音频入队**之前**喂，且用 audio_play_stamp() 取时间戳：
     * 那是这批数据将来在时间轴上排的位置，口型帧打上它才能真正对上声音。
     * （早期按"先喂后取"写，结果所有口型整体偏早 200ms，嘴先动、声音后到。）*/
    if (g_av) {
        av_feed_pcm(g_av, pcm, samples, audio_play_stamp(&g_play));
    }

    /* 云端比播放快，这里会阻塞等缓冲，正常现象 */
    audio_play_write(&g_play, pcm, samples);
}

static void cb_error(int code, const char *msg, void *user)
{
    (void)user;
    printf("[错误] code=%d %s\n", code, msg ? msg : "");
    fflush(stdout);
}

static const cc_callbacks_t g_cbs = {
    .on_state = cb_state,
    .on_asr   = cb_asr,
    .on_reply = cb_reply,
    .on_audio = cb_audio,
    .on_error = cb_error,
    .user     = NULL,
};

/* =============================================================================
 * 麦克风回调：直接喂云端
 * ========================================================================== */
static void on_capture(const int16_t *samples, size_t n, void *user)
{
    (void)user;
    cc_feed_pcm(g_cc, samples, n);
}

/* =============================================================================
 * main
 * ========================================================================== */
int main(int argc, char **argv)
{
    /* --wav <file>：用 WAV 文件模拟麦克风（回归测试 / 无人值守验证）
     * 走的是和真实麦克风完全相同的链路（cc_feed_pcm → Opus → 云端）*/
    const char *wav_path = NULL;
    for (int i = 1; i < argc - 1; i++) {
        if (strcmp(argv[i], "--wav") == 0) wav_path = argv[i + 1];
    }
    signal(SIGINT, on_sig);
    signal(SIGTERM, on_sig);
    setvbuf(stdout, NULL, _IONBF, 0);

    printf("==========================================================\n");
    printf(" DEMO-030 语音对话（纯 C / 云端一体化）\n");
    printf("==========================================================\n");
    printf(" 麦克风 : %s  %dHz 单声道\n", CAP_DEVICE, SAMPLE_RATE);
    printf(" 喇叭   : %s\n", PLAY_DEVICE);
    printf(" 云端   : VAD + ASR + LLM + TTS 全在云端\n");
    printf(" 编码   : 上行 Opus 10ms/24kbps，下行 Opus\n");
    printf("==========================================================\n\n");

    /* ---- 1. 先起播放（云端一旦回音频就能立刻出声）---- */
    printf("[1] 打开喇叭…\n");
    if (audio_play_start(&g_play, PLAY_DEVICE, SAMPLE_RATE, 3000) != 0) {
        printf("    ✗ 播放设备打不开\n");
        return 1;
    }
    printf("    ✓ 就绪\n");

    /* ---- 1.5 起界面（失败不影响语音，自动退化为纯音频模式）---- */
    printf("\n[1.5] 打开数字人界面…\n");
    g_av = av_start();
    if (g_av) {
        av_set_position_source(play_pos_cb, NULL);
        printf("    ✓ 界面就绪\n");
    } else {
        printf("    - 跳过（无 X 或无素材，仅语音模式）\n");
    }

    /* ---- 2. 连云端 ---- */
    printf("\n[2] 连接云端（DMP 注册 + WebSocket）…\n");
    g_cc = cc_start(&g_cbs);
    if (!g_cc) {
        printf("    ✗ 云端连接失败\n");
        audio_play_stop(&g_play);
        return 1;
    }
    printf("    ✓ 云端就绪\n");

    /* ---- 3. 音频输入：真实麦克风 or WAV 文件 ---- */
    audio_cap_t cap;
    memset(&cap, 0, sizeof(cap));
    FILE *wav_fp = NULL;
    if (wav_path) {
        printf("\n[3] 用 WAV 模拟麦克风: %s\n", wav_path);
        wav_fp = fopen(wav_path, "rb");
        if (!wav_fp) { printf("    ✗ 打不开\n"); cc_stop(g_cc);
                       audio_play_stop(&g_play); return 1; }
        unsigned char hdr[44];
        if (fread(hdr, 1, 44, wav_fp) != 44) {
            printf("    ✗ WAV 头不完整\n");
            fclose(wav_fp); cc_stop(g_cc); audio_play_stop(&g_play); return 1;
        }
        printf("    ✓ 就绪（按实时速率喂入）\n");
    } else {
        printf("\n[3] 打开麦克风…\n");
        if (audio_cap_start(&cap, CAP_DEVICE, SAMPLE_RATE, CAP_BLOCK_MS,
                            on_capture, NULL) != 0) {
            printf("    ✗ 麦克风打不开\n");
            cc_stop(g_cc);
            audio_play_stop(&g_play);
            return 1;
        }
        printf("    ✓ 就绪\n");
    }

    printf("\n==========================================================\n");
    printf(" 开始对话，直接对着麦克风说话即可（Ctrl+C 退出）\n");
    printf("==========================================================\n");

    /* ---- 4. 主循环：实时状态显示 ---- */
    int tick = 0;
    int wav_loops = 0;
    int16_t wav_buf[320];        /* 20ms */
    while (g_run) {
        if (wav_fp) {
            size_t got = fread(wav_buf, 1, sizeof(wav_buf), wav_fp);
            if (got < sizeof(wav_buf)) {
                memset((char *)wav_buf + got, 0, sizeof(wav_buf) - got);
                if (got == 0) {
                    wav_loops++;
                    printf("\n[WAV] 第 %d 遍播完，重播\n", wav_loops);
                    fseek(wav_fp, 44, SEEK_SET);
                }
            }
            cc_feed_pcm(g_cc, wav_buf, sizeof(wav_buf) / sizeof(int16_t));
            usleep(20 * 1000);    /* 20ms 实时节奏 */
        } else {
            usleep(100 * 1000);
        }
        tick++;
        /* 状态行只在终端里刷新（\r 覆盖）。
         * 重定向到文件时不要刷，否则日志会被冲成一堆 \r 垃圾。*/
        static int is_tty = -1;
        if (is_tty < 0) is_tty = isatty(STDOUT_FILENO) ? 1 : 0;
        /* GUI level updates must also work in the detached/background launch. */
        if (g_av && tick % 10 == 0) av_set_mic_level(g_av, audio_cap_level());
        if (is_tty && tick % 10 == 0) {
            float lv = audio_cap_level();
            size_t pend = audio_play_pending(&g_play);
            int bars = (int)(lv * 20);
            char meter[24];
            memset(meter, ' ', 20);
            meter[20] = 0;
            for (int i = 0; i < bars && i < 20; i++) meter[i] = '#';

            printf("\r[状态] %-6d 音量[%s] 播放缓冲%4zums ASR增量%d 回答增量%d",
                   (int)cc_state(g_cc), meter,
                   pend * 1000 / SAMPLE_RATE, g_asr_n, g_reply_n);
            fflush(stdout);
        }
    }

    printf("\n\n==========================================================\n");
    printf(" 退出，统计\n");
    printf("==========================================================\n");
    cc_stats_t st;
    cc_get_stats(g_cc, &st);
    printf(" 上行音频 : %lu 字节 / %lu 帧\n", st.audio_tx_bytes, st.frames_tx);
    printf(" 下行音频 : %lu 字节\n", st.audio_rx_bytes);
    printf(" ASR 增量 : %lu 次\n", st.asr_deltas);
    printf(" LLM 增量 : %lu 次\n", st.llm_deltas);
    printf(" 段内欠载 : %d 次（真正影响听感的断续）\n", g_play.underruns);
    printf(" 段间空隙 : %d 次（句间正常排空，无害）\n", g_play.idle_gaps);
    printf("==========================================================\n");

    /* ---- 5. 收尾 ---- */
    if (wav_fp) fclose(wav_fp);
    if (!wav_path) audio_cap_stop(&cap);
    cc_stop(g_cc);
    audio_play_stop(&g_play);
    if (g_av) av_stop(g_av);        /* 界面最后关，让最后一句能看到嘴闭上 */
    return 0;
}
