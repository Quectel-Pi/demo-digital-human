/*
 * avatar.c — 数字人显示 + 真实音频驱动的口型
 *
 * 架构
 * ====
 *   渲染线程（本文件）         播放线程（audio_io.c）
 *   ─────────────────          ─────────────────────
 *   每 40ms 一帧               每 50ms 写一个 chunk
 *   查 audio_play_position()   累加 written / 采样 queued
 *   取该位置对应的口型          在 audio_play_write() 里
 *   发布到 HTTP 状态桥         给每个 20ms 口型帧打上 written 标签
 *
 * 渲染方式
 * ========
 * 本文件只负责**口型计算 + 状态发布**，实际绘制由浏览器完成：
 *   1. av_start() 起一个只监听 127.0.0.1:8765 的 HTTP 服务（见 avatar_web.c），
 *      对外提供 /api/state（JSON）和 web/ 目录下的静态资源；
 *   2. Firefox kiosk 打开该地址，用 three.js + @pixiv/three-vrm 渲染 VRM 模型；
 *   3. 每帧从 /api/state 拿到 openness/wide/round，映射到 VRM 的表情变形。
 * 所以这里不需要 X11、不需要字体、不需要任何本地贴图。
 *
 * 为什么不在板上用 CPU 绘制
 * ------------------------
 * RK3576 用 CPU 做整屏 1280x800 的精灵合成，每帧要几毫秒到十几毫秒，
 * 美术资源还有上百 MB；改用 GPU（Mali-G51）跑 WebGL 后帧率稳定、
 * 资源只需一个 VRM 文件，形象也能直接换成客户定制的模型。
 *
 * 为什么口型要跟 audio_play_position() 而不是跟"写入时刻"
 * -------------------------------------------------------
 * 写之前要先攒 prefill，PulseAudio 侧又有 tlength 缓冲，所以写入时刻
 * 比真正出声最多早几百毫秒。若口型跟着写入走，嘴会提前张开、
 * 声音还没出来，一眼就看出是假的。用 written - queued 拿到的才是
 * 「此刻喇叭正在放第几个采样」，口型跟它走才真正同步。
 *
 * 口型怎么算
 * ----------
 * 每 20ms 一帧，从真实 PCM 算三个参数（与 Python 版算法一致）：
 *   openness = 对数压缩的音量包络   → 张嘴多大
 *   wide     = 过零率               → 齿音/前元音，嘴更扁
 *   round    = 低频(<500Hz)能量占比 → 圆唇音，嘴更圆
 * 因为是"按音频算"而不是"按时间猜"，声音一停嘴立刻停。
 */
#include "avatar.h"
#include "avatar_web.h"

#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

/* =============================================================================
 * 口型帧队列
 * ========================================================================== */
typedef struct {
    float openness;
    float wide;
    float round_;
    unsigned long long stamp;   /* 在播放时间轴上的起始采样位置 */
} viseme_t;

#define VISEME_QCAP 8192        /* 8192 * 20ms ≈ 164 秒，足够 */

struct avatar_s {
    /* 口型队列（生产者=播放线程，消费者=渲染线程）*/
    pthread_mutex_t qlock;
    viseme_t        q[VISEME_QCAP];
    int             qhead, qtail;

    /* 最近的 PCM 残余（用于凑满 20ms 一帧）*/
    int16_t  carry[AV_VISEME_HOP];
    size_t   carry_n;

    /* 状态 */
    _Atomic int running;
    volatile av_state_t state;
    pthread_t thread;
    avatar_web_t *web;          /* HTTP 状态桥 */

    /* 文本（受 tlock 保护）*/
    pthread_mutex_t tlock;
    char question[512];
    char answer[1024];

    double   t0;
    int      frame_ms;
    unsigned long frames;

    /* 界面上显示的指标 */
    volatile float mic_level;
};

/* =============================================================================
 * 小工具
 * ========================================================================== */
static double now_s(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + (double)ts.tv_nsec * 1e-9;
}

/* =============================================================================
 * 播放进度来源
 *
 * avatar 模块不应该知道 audio_play_t 的存在（否则两个模块耦合，测试困难）。
 * 改成注册一个「取当前播放位置」的回调，由 main 在启动时接上。
 * ========================================================================== */
static av_posfn_t      g_posfn;
static void           *g_posuser;

void av_set_position_source(av_posfn_t fn, void *user)
{
    g_posfn = fn;
    g_posuser = user;
}

static unsigned long long current_pos(void)
{
    return g_posfn ? g_posfn(g_posuser) : 0;
}

/* =============================================================================
 * 口型计算
 * ========================================================================== */
/* 把 20ms 的 PCM 换算成一个口型帧。
 * 三个参数的映射区间沿用 Python 版实测值，直接搬过来保持观感一致。*/
static void pcm_to_viseme(const int16_t *s, size_t n,
                          float *openness, float *wide, float *round_)
{
    if (n == 0) { *openness = 0; *wide = 0; *round_ = 0; return; }

    /* 1) RMS → 张嘴。用对数压缩，轻声也有微动、大声不爆。
     *
     * ⚠️ 参考值不能写死。最初固定 0.03（约 -30dBFS），结果云端 TTS 音量
     * 偏小时 open 长期是 0.0，说话过程中嘴频繁闭上、"一抽一抽"。
     * 改成自适应：用最近若干帧的峰值做参考，自动适配不同音色的 TTS。
     */
    static double peak_ref = 0.0;
    double sum = 0.0;
    for (size_t i = 0; i < n; i++) {
        double v = (double)s[i] / 32768.0;
        sum += v * v;
    }
    double rms = sqrt(sum / (double)n);

    /* 峰值缓慢衰减（每秒衰减约 3%），避免一句大声之后长时间失去灵敏度 */
    if (rms > peak_ref) peak_ref = rms;
    else peak_ref = peak_ref * 0.97 + rms * 0.03;
    if (peak_ref < 0.012) peak_ref = 0.012;      /* 下限，防止静音时噪声被放大 */

    /* 以峰值的 45% 作为"满张口"基准：正常说话能到 0.7~1.0，
     * 轻声也有 0.2~0.4 的可见动作，不会一直闭嘴。*/
    double ref = peak_ref;
    double op = log1p(rms * 9.0 / ref) / log(10.0);
    if (op > 1.0) op = 1.0;
    if (op < 0.0) op = 0.0;

    /* 2) 过零率 → 扁平度（齿音/前元音 ZCR 高）*/
    int zc = 0;
    for (size_t i = 1; i < n; i++) {
        if ((s[i - 1] < 0) != (s[i] < 0)) zc++;
    }
    double z = (double)zc / (double)n;
    double wd = (z - 0.02) / 0.30;
    if (wd > 1.0) wd = 1.0;
    if (wd < 0.0) wd = 0.0;

    /* 3) 低频能量占比 → 圆唇度。
     * 用一阶 IIR 低通（截止 ~500Hz）近似，递推即可，成本极低。*/
    double a = 2.0 * M_PI * 500.0 / 16000.0;
    if (a > 0.99) a = 0.99;
    double lp = 0.0, lp_e = 0.0, full_e = 0.0;
    for (size_t i = 0; i < n; i++) {
        double v = (double)s[i] / 32768.0;
        lp += a * (v - lp);
        lp_e += lp * lp;
        full_e += v * v;
    }
    double ratio = (full_e > 1e-12) ? (lp_e / full_e) : 0.0;
    double rd = (ratio - 0.30) / 0.55;
    if (rd > 1.0) rd = 1.0;
    if (rd < 0.0) rd = 0.0;

    *openness = (float)op;
    *wide     = (float)wd;
    *round_   = (float)rd;
}

/* =============================================================================
 * 对外：喂 PCM（由播放线程调用）
 * ========================================================================== */
void av_feed_pcm(avatar_t *av, const int16_t *pcm, size_t samples,
                 unsigned long long base_stamp)
{
    if (!av || !pcm || samples == 0) return;

    pthread_mutex_lock(&av->qlock);

    size_t off = 0;
    /* 先把上次的残余补满 */
    while (off < samples) {
        size_t need = AV_VISEME_HOP - av->carry_n;
        size_t take = (samples - off < need) ? (samples - off) : need;
        memcpy(av->carry + av->carry_n, pcm + off, take * sizeof(int16_t));
        av->carry_n += take;
        off += take;

        if (av->carry_n < AV_VISEME_HOP) break;

        /* 凑满 20ms，产出一帧口型 */
        float op, wd, rd;
        pcm_to_viseme(av->carry, AV_VISEME_HOP, &op, &wd, &rd);

        int next = (av->qhead + 1) % VISEME_QCAP;
        if (next == av->qtail) {
            /* 队列满（渲染端跟不上），丢最老的，保证不阻塞播放线程 */
            av->qtail = (av->qtail + 1) % VISEME_QCAP;
        }
        av->q[av->qhead].openness = op;
        av->q[av->qhead].wide     = wd;
        av->q[av->qhead].round_   = rd;
        /* 这帧对应 base_stamp 之后第 (off - HOP) 个采样开始的 20ms 窗口，
         * 也就是它在播放时间轴上的绝对位置。*/
        av->q[av->qhead].stamp    = base_stamp
                                  + (unsigned long long)(off - AV_VISEME_HOP);
        av->qhead = next;
        av->carry_n = 0;
    }

    pthread_mutex_unlock(&av->qlock);
}

void av_audio_done(avatar_t *av)
{
    if (!av) return;
    pthread_mutex_lock(&av->qlock);
    if (av->carry_n > 0) {
        /* 尾巴不足 20ms，直接丢掉；同时让口型回到静音 */
        av->carry_n = 0;
    }
    /* 不再清空队列 —— 队列里剩下的帧正是「已写进缓冲、还没播出来」的部分，
     * 必须留着让渲染线程按播放位置取，否则嘴会提前闭上。*/
    pthread_mutex_unlock(&av->qlock);
}

void av_flush(avatar_t *av)
{
    if (!av) return;
    pthread_mutex_lock(&av->qlock);
    av->qtail = av->qhead = 0;
    av->carry_n = 0;
    pthread_mutex_unlock(&av->qlock);
}

/* 按播放位置取当前该显示的口型。
 * 播放位置 pos 是「已播出的采样数」，口型帧的 stamp 也是同一坐标系，
 * 所以直接换算成帧号即可，不需要任何时间推算。*/
static int pick_viseme(avatar_t *av, unsigned long long pos,
                       float *op, float *wd, float *rd)
{
    int got = 0;
    pthread_mutex_lock(&av->qlock);
    /* Compare actual sample ranges, not rounded frame buckets. A queued
     * future frame must never open the mouth before its audio is playing. */
    while (av->qtail != av->qhead) {
        unsigned long long st = av->q[av->qtail].stamp;
        if (pos >= st && pos - st >= AV_VISEME_HOP) {
            av->qtail = (av->qtail + 1) % VISEME_QCAP;
        } else break;
    }

    if (av->qtail != av->qhead) {
        viseme_t *v = &av->q[av->qtail];
        if (pos >= v->stamp) {
            *op = v->openness;
            *wd = v->wide;
            *rd = v->round_;
            got = 1;
        }
    }
    pthread_mutex_unlock(&av->qlock);
    return got;
}

/* =============================================================================
 * 渲染一帧：打包状态发给浏览器
 * ========================================================================== */
static void render_frame(avatar_t *av, unsigned long long pos)
{
    avatar_web_state_t snapshot = {0};

    /* State/text/mic writers use the same lock. The HTTP worker receives one
     * complete copy, never mixed fields/strings. */
    pthread_mutex_lock(&av->tlock);
    snapshot.state = av->state;
    snapshot.mic = av->mic_level;
    memcpy(snapshot.question, av->question, sizeof(snapshot.question));
    memcpy(snapshot.answer, av->answer, sizeof(snapshot.answer));
    /* Network TTS completion may precede the speaker draining its queue.
     * Keep driving those real playback frames after the cloud says idle. */
    if (snapshot.state != AV_STATE_LISTEN &&
        pick_viseme(av, pos, &snapshot.openness, &snapshot.wide,
                    &snapshot.round_))
        snapshot.state = AV_STATE_SPEAK;
    pthread_mutex_unlock(&av->tlock);

    avatar_web_publish(av->web, &snapshot);
}

static void *render_thread(void *arg)
{
    avatar_t *av = (avatar_t *)arg;
    struct timespec ts;

    fprintf(stderr, "[AVWEB] 渲染线程启动\n");

    while (av->running) {
        render_frame(av, current_pos());
        av->frames++;

        ts.tv_sec  = av->frame_ms / 1000;
        ts.tv_nsec = (long)(av->frame_ms % 1000) * 1000000L;
        nanosleep(&ts, NULL);
    }
    fprintf(stderr, "[AVWEB] 渲染线程退出，共 %lu 帧\n", av->frames);
    return NULL;
}

/* =============================================================================
 * 启动 / 关闭
 * ========================================================================== */
avatar_t *av_start(void)
{
    /* 只有一种渲染方式：WebGL。必须给出静态资源目录。 */
    const char *web_root = getenv("DEMO030_WEB_ROOT");
    if (!web_root || !*web_root) {
        fprintf(stderr, "[AVWEB] ✗ 未设置 DEMO030_WEB_ROOT，界面不可用\n");
        fprintf(stderr, "[AVWEB]   例: DEMO030_WEB_ROOT=/userdata/demo030/web\n");
        return NULL;
    }
    if (getenv("DEMO030_NO_GUI")) {
        fprintf(stderr, "[AVWEB] DEMO030_NO_GUI 已设置，跳过界面\n");
        return NULL;
    }

    avatar_t *av = calloc(1, sizeof(*av));
    if (!av) {
        fprintf(stderr, "[AVWEB] 内存分配失败\n");
        return NULL;
    }

    int rc = pthread_mutex_init(&av->qlock, NULL);
    if (!rc) rc = pthread_mutex_init(&av->tlock, NULL);
    if (rc) {
        fprintf(stderr, "[AVWEB] 互斥量初始化失败: %s\n", strerror(rc));
        free(av);
        return NULL;
    }

    av->t0 = now_s();
    av->state = AV_STATE_IDLE;
    av->frame_ms = 40;                 /* 25fps，口型足够顺滑且省 CPU */
    av->running = 1;

    av->web = avatar_web_start(web_root);
    if (!av->web) {
        fprintf(stderr, "[AVWEB] HTTP 状态桥启动失败（端口 8765 被占用？）\n");
        av->running = 0;
        pthread_mutex_destroy(&av->tlock);
        pthread_mutex_destroy(&av->qlock);
        free(av);
        return NULL;
    }

    if (pthread_create(&av->thread, NULL, render_thread, av) != 0) {
        fprintf(stderr, "[AVWEB] 渲染线程创建失败\n");
        av->running = 0;
        avatar_web_stop(av->web);
        pthread_mutex_destroy(&av->tlock);
        pthread_mutex_destroy(&av->qlock);
        free(av);
        return NULL;
    }

    fprintf(stderr, "[AVWEB] ✓ 界面已启动 %dx%d @%dfps\n",
            AV_W, AV_H, 1000 / av->frame_ms);
    return av;
}

void av_stop(avatar_t *av)
{
    if (!av) return;
    av->running = 0;
    pthread_join(av->thread, NULL);
    avatar_web_stop(av->web);

    pthread_mutex_destroy(&av->qlock);
    pthread_mutex_destroy(&av->tlock);
    free(av);
}

/* =============================================================================
 * 状态 / 文本 / 电平
 * ========================================================================== */
void av_set_state(avatar_t *av, av_state_t st)
{
    if (!av) return;
    pthread_mutex_lock(&av->tlock);
    av->state = st;
    pthread_mutex_unlock(&av->tlock);
}

void av_set_text(avatar_t *av, const char *q, const char *a)
{
    if (!av) return;
    pthread_mutex_lock(&av->tlock);
    if (q) snprintf(av->question, sizeof(av->question), "%s", q);
    if (a) snprintf(av->answer, sizeof(av->answer), "%s", a);
    pthread_mutex_unlock(&av->tlock);
}

void av_set_mic_level(avatar_t *av, float level)
{
    if (!av) return;
    pthread_mutex_lock(&av->tlock);
    av->mic_level = level;
    pthread_mutex_unlock(&av->tlock);
}

void av_tick(avatar_t *av) { (void)av; }
