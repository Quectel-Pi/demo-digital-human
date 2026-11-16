/* =============================================================================
 * audio_io.h —— ALSA 录音 + 播放
 *
 * 板上 ES8389 codec（card0 / plughw:0,0）录音和播放共用同一个设备。
 *
 * 设计要点：
 *   - 录音在独立线程跑，回调里只做「拷进环形缓冲」，绝不阻塞；
 *   - 播放也在独立线程，喂 PCM 的线程只管往环形缓冲写，不直接碰 ALSA；
 *   - 两边都用 16kHz / 单声道 / S16_LE，与云端 Opus 编码完全对齐，
 *     省掉任何重采样（RK3576 上重采样既费 CPU 又引入延迟）。
 * ========================================================================== */

#ifndef DEMO030_AUDIO_IO_H
#define DEMO030_AUDIO_IO_H

#include <stdint.h>
#include <stddef.h>
#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ---- 环形缓冲（单生产者单消费者，无锁）---- */
typedef struct {
    int16_t *buf;
    size_t   cap;        /* 采样数 */
    volatile size_t w;   /* 写位置 */
    volatile size_t r;   /* 读位置 */
} ring_t;

int  ring_init(ring_t *rb, size_t capacity_samples);
void ring_free(ring_t *rb);
size_t ring_avail(const ring_t *rb);      /* 可读采样数 */
size_t ring_space(const ring_t *rb);      /* 可写采样数 */
size_t ring_write(ring_t *rb, const int16_t *src, size_t n);
size_t ring_read(ring_t *rb, int16_t *dst, size_t n);
void ring_clear(ring_t *rb);

/* ---- 录音 ---- */
typedef void (*audio_capture_cb_t)(const int16_t *samples, size_t n,
                                   void *user);

typedef struct {
    char            device[64];
    unsigned int    sample_rate;
    unsigned int    channels;
    unsigned int    block_ms;      /* 每次回调的块长（ms）*/
    audio_capture_cb_t cb;
    void           *user;

    /* 内部 */
    void           *pcm;
    pthread_t       thread;
    volatile int    running;
    int             err;
} audio_cap_t;

/* 返回 0 成功 */
int  audio_cap_start(audio_cap_t *cap, const char *device,
                     unsigned int sample_rate, unsigned int block_ms,
                     audio_capture_cb_t cb, void *user);
void audio_cap_stop(audio_cap_t *cap);
/* 当前输入电平 0..1（给 UI 用） */
float audio_cap_level(void);

/* ---- 播放 ---- */
typedef struct {
    char            device[64];
    unsigned int    sample_rate;
    unsigned int    channels;

    void           *pcm;          /* snd_pcm_t* */

    ring_t          ring;
    pthread_t       thread;
    volatile int    running;
    volatile int    draining;     /* 数据源已结束，放完缓冲就退出 */
    int             err;
    volatile int    underruns;        /* 段内欠载（真的会听到断续，要关注）*/
    volatile int    idle_gaps;        /* 段间空隙（一句话放完等下一句，正常）*/

    /* ---- 播放进度（给数字人口型对齐用）----
     *
     * 口型必须跟「真正从喇叭出来的声音」对齐，不能跟「写进缓冲的时刻」对齐。
     * 因为我们在写之前会先攒 200ms（prefill），再加上 ALSA 硬件缓冲 600ms，
     * 写入时刻比出声时刻最多早 800ms。如果口型跟着写入走，嘴会提前近 1 秒
     * 张开、声音却还没出来，非常假。
     *
     * written：累计写入 ALSA 的采样数（单调递增，跨语音段不清零）
     * queued ：此刻还堆积在硬件缓冲里没播出去的采样数（snd_pcm_delay 值）
     *
     * 实际出声位置 = written - queued。两个量都在同一坐标系里，
     * 所以口型帧只要打上「写入时刻的 written 值」作为标签，
     * 渲染时用 written-queued 去查，就能精确对上。*/
    volatile unsigned long long written;
    volatile long long          queued;
} audio_play_t;

int  audio_play_start(audio_play_t *pl, const char *device,
                      unsigned int sample_rate, size_t ring_ms);
void audio_play_stop(audio_play_t *pl);
/* 写入 PCM（阻塞到有空位，返回写入采样数）*/
size_t audio_play_write(audio_play_t *pl, const int16_t *pcm, size_t n);

/* 清空缓冲（打断时用）*/
void audio_play_flush(audio_play_t *pl);

/* ring 缓冲里还有多少采样没写进 ALSA（给 UI 显示 / 调试用）*/
size_t audio_play_pending(audio_play_t *pl);

/* 累计已写入 ALSA 的采样数。在 audio_play_write() **之前**调用，
 * 得到的就是这批数据在播放时间轴上的起点。*/
unsigned long long audio_play_written(const audio_play_t *pl);

/* 累计已写入 ALSA 的采样数 + 还排在 ring 里没写的采样数。
 *
 * 这个和就是「这批新数据将来在播放时间轴上排第几」。
 * 口型帧必须以它为时间戳，才能和真正出声的时刻对上。*/
unsigned long long audio_play_stamp(audio_play_t *pl);

/* 此刻真正从扬声器出来的采样位置（= written - 排队延迟）。
 * 数字人渲染线程每帧查一次，用来决定该显示哪个口型。*/
unsigned long long audio_play_position(audio_play_t *pl);

/* 取「此刻正在播放的那段 PCM」作为 AEC 参考信号。
 *
 * 为什么需要这个：AEC 要消掉喇叭回声，必须知道**现在喇叭正在放什么**，
 * 而且要和麦克风采到的那一帧严格时间对齐。差一点就会消不干净甚至误消人声。
 *
 * 实现见 audio_io.c：维护一份最近播放数据的环形镜像，按 pos - latency
 * 回溯取值。n 通常是 AEC_FRAME(160)，不够的部分补零。
 * 返回实际填充的采样数。*/
size_t audio_play_ref_pcm(audio_play_t *pl, int16_t *dst, size_t n);

#ifdef __cplusplus
}
#endif

#endif
