/* =============================================================================
 * audio_io.c —— PulseAudio 录音 + 播放实现
 * ========================================================================== */

#include "audio_io.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <math.h>

/* =============================================================================
 * PulseAudio 最小声明
 *
 * 为什么不用 ALSA 直接播：见文件末尾「播放」一节的说明。
 * 板上跑着 pipewire-pulse，参考项目（Mamboe_App）就是用 pa_simple_write
 * 流式播放的，从不欠载。这里只声明用到的那几个符号，避免拉进 libpulse 头
 * 文件依赖（板上没有 dev 包，这是本项目一贯的处理方式）。
 * ========================================================================== */
typedef struct pa_simple pa_simple;

enum {
    PA_STREAM_NODIRECTION = 0,
    PA_STREAM_PLAYBACK    = 1,
    PA_STREAM_RECORD      = 2,
};

enum { PA_SAMPLE_S16LE = 3 };          /* pa_sample_format_t */

enum { PA_CHANNEL_POSITION_MONO = 1 };

typedef struct {
    int      format;
    uint32_t rate;
    uint8_t  channels;
} pa_sample_spec;

typedef struct {
    uint8_t  channels;
    int      map[32];
} pa_channel_map;

typedef struct {
    uint32_t maxlength;
    uint32_t tlength;
    uint32_t prebuf;
    uint32_t minreq;
    uint32_t fragsize;
} pa_buffer_attr;

extern pa_simple *pa_simple_new(const char *server, const char *name,
                                int dir, const char *dev,
                                const char *stream_name,
                                const pa_sample_spec *ss,
                                const pa_channel_map *map,
                                const pa_buffer_attr *attr, int *error);
extern void pa_simple_free(pa_simple *s);
extern int  pa_simple_write(pa_simple *s, const void *data, size_t bytes,
                            int *error);
extern int  pa_simple_read(pa_simple *s, void *data, size_t bytes, int *error);
extern int  pa_simple_drain(pa_simple *s, int *error);
extern int  pa_simple_flush(pa_simple *s, int *error);
extern int  pa_simple_get_latency(pa_simple *s, int *error);

/* =============================================================================
 * 环形缓冲
 * ========================================================================== */
int ring_init(ring_t *rb, size_t capacity_samples)
{
    if (!rb || capacity_samples == 0) return -1;
    rb->buf = (int16_t *)calloc(capacity_samples, sizeof(int16_t));
    if (!rb->buf) return -1;
    rb->cap = capacity_samples;
    rb->w = rb->r = 0;
    return 0;
}

void ring_free(ring_t *rb)
{
    if (!rb) return;
    free(rb->buf);
    rb->buf = NULL;
    rb->cap = rb->w = rb->r = 0;
}

size_t ring_avail(const ring_t *rb)
{
    size_t w = rb->w, r = rb->r;
    return (w >= r) ? (w - r) : (rb->cap - r + w);
}

size_t ring_space(const ring_t *rb)
{
    /* 留 1 个空位区分满/空 */
    return rb->cap - 1 - ring_avail(rb);
}

size_t ring_write(ring_t *rb, const int16_t *src, size_t n)
{
    size_t space = ring_space(rb);
    if (n > space) n = space;
    if (n == 0) return 0;

    size_t w = rb->w;
    size_t first = rb->cap - w;
    if (first > n) first = n;
    memcpy(rb->buf + w, src, first * sizeof(int16_t));
    if (n > first) {
        memcpy(rb->buf, src + first, (n - first) * sizeof(int16_t));
    }
    rb->w = (w + n) % rb->cap;
    return n;
}

size_t ring_read(ring_t *rb, int16_t *dst, size_t n)
{
    size_t avail = ring_avail(rb);
    if (n > avail) n = avail;
    if (n == 0) return 0;

    size_t r = rb->r;
    size_t first = rb->cap - r;
    if (first > n) first = n;
    memcpy(dst, rb->buf + r, first * sizeof(int16_t));
    if (n > first) {
        memcpy(dst + first, rb->buf, (n - first) * sizeof(int16_t));
    }
    rb->r = (r + n) % rb->cap;
    return n;
}

void ring_clear(ring_t *rb)
{
    rb->r = rb->w = 0;
}

/* =============================================================================
 * 录音
 * ========================================================================== */

/*: 全局输入电平，渲染线程读它画音量条（0..1）*/
static volatile float g_cap_level = 0.0f;

float audio_cap_level(void) { return g_cap_level; }

static void *cap_thread(void *arg)
{
    audio_cap_t *cap = (audio_cap_t *)arg;
    pa_simple *stream = (pa_simple *)cap->pcm;
    size_t block = (size_t)(cap->sample_rate / 1000 * cap->block_ms);
    int16_t *buf = (int16_t *)malloc(block * sizeof(int16_t));
    if (!buf) {
        cap->err = -ENOMEM;
        return NULL;
    }

    fprintf(stderr, "[CAP] 采集线程启动（PulseAudio, block=%zu/%.0fms）\n",
            block, block * 1000.0 / cap->sample_rate);

    while (cap->running) {
        int err = 0;
        /* pa_simple_read 会阻塞到凑满 block 字节，天然按实时节奏返回，
         * 不需要自己算时间。失败（服务重启等）就重建流。 */
        if (pa_simple_read(stream, buf, block * sizeof(int16_t), &err) < 0) {
            fprintf(stderr, "[CAP] pa_simple_read 失败: code=%d，重建流\n", err);
            pa_simple_free(stream);

            pa_sample_spec ss;
            memset(&ss, 0, sizeof(ss));
            ss.format   = PA_SAMPLE_S16LE;
            ss.rate     = cap->sample_rate;
            ss.channels = 1;
            pa_buffer_attr ba;
            memset(&ba, 0, sizeof(ba));
            ba.maxlength = (uint32_t)-1;
            ba.tlength   = (uint32_t)-1;
            ba.prebuf    = (uint32_t)-1;
            ba.minreq    = (uint32_t)-1;
            ba.fragsize  = (uint32_t)(block * sizeof(int16_t) * 2);

            /* 录音必须从 AEC 的 Echo-Cancel Source（mic-ns）取，与播放端的
             * mic-ns-sink 配对。从 alsa_input 直取拿到的是**未消回声**的裸麦克风，
             * 会把喇叭播放声一起录进去，导致云端误判成用户插话（自打断）。
             * 详见 audio_play_start() 里那段说明。*/
            int e2 = 0;
            stream = pa_simple_new(NULL, "DEMO030", PA_STREAM_RECORD,
                                   "mic-ns", "mic", &ss, NULL, &ba, &e2);
            if (!stream) {
                fprintf(stderr, "[CAP] ⚠ 打不开 AEC source(mic-ns) code=%d，"
                                "改用默认麦克风 —— 会自打断\n", e2);
                stream = pa_simple_new(NULL, "DEMO030", PA_STREAM_RECORD,
                                       NULL, "mic", &ss, NULL, &ba, &e2);
            }
            if (!stream) {
                /* 注意：这是在线程里，不能 return -1，只能退出循环 */
                fprintf(stderr, "[CAP] ✗ 重建录音流失败 (code=%d)\n", e2);
                cap->err = e2;
                break;
            }
            cap->pcm = stream;
            continue;
        }

        /* 音量电平（给 UI 和口型无关，纯显示用）*/
        double sum = 0.0;
        for (size_t i = 0; i < block; i++) {
            double v = buf[i] / 32768.0;
            sum += v * v;
        }
        double rms = sqrt(sum / (double)block);
        g_cap_level = (float)(rms > 1.0 ? 1.0 : rms);

        if (cap->cb) cap->cb(buf, block, cap->user);
    }

    free(buf);
    fprintf(stderr, "[CAP] 采集线程退出\n");
    return NULL;
}

int audio_cap_start(audio_cap_t *cap, const char *device,
                    unsigned int sample_rate, unsigned int block_ms,
                    audio_capture_cb_t cb, void *user)
{
    (void)device;      /* PA 用默认 source */
    memset(cap, 0, sizeof(*cap));
    snprintf(cap->device, sizeof(cap->device), "%s", "pulse-default");
    cap->sample_rate = sample_rate ? sample_rate : 16000;
    cap->channels = 1;
    cap->block_ms = block_ms ? block_ms : 20;
    cap->cb = cb;
    cap->user = user;

    pa_sample_spec ss;
    memset(&ss, 0, sizeof(ss));
    ss.format   = PA_SAMPLE_S16LE;
    ss.rate     = cap->sample_rate;
    ss.channels = 1;

    /* fragsize 控制每次 read 返回多少：给 2 倍 block，让 read 更少阻塞、
     * 但也不会一次吞太多导致延迟变高。*/
    size_t block = (size_t)(cap->sample_rate / 1000 * cap->block_ms);
    pa_buffer_attr ba;
    memset(&ba, 0, sizeof(ba));
    ba.maxlength = (uint32_t)-1;
    ba.tlength   = (uint32_t)-1;
    ba.prebuf    = (uint32_t)-1;
    ba.minreq    = (uint32_t)-1;
    ba.fragsize  = (uint32_t)(block * sizeof(int16_t) * 2);

    int err = 0;
    pa_simple *stream = pa_simple_new(NULL, "DEMO030", PA_STREAM_RECORD,
                                      NULL, "mic", &ss, NULL, &ba, &err);
    if (!stream) {
        fprintf(stderr, "[CAP] ✗ 打不开 PulseAudio 录音流 (code=%d)\n", err);
        return -1;
    }

    cap->pcm = stream;
    cap->running = 1;
    if (pthread_create(&cap->thread, NULL, cap_thread, cap) != 0) {
        pa_simple_free(stream);
        cap->pcm = NULL;
        return -1;
    }

    fprintf(stderr, "[CAP] ✓ PulseAudio 录音流就绪 %uHz/1ch/S16\n",
            cap->sample_rate);
    return 0;
}

void audio_cap_stop(audio_cap_t *cap)
{
    if (!cap->running) return;
    cap->running = 0;
    pthread_join(cap->thread, NULL);
    if (cap->pcm) {
        pa_simple_free((pa_simple *)cap->pcm);
        cap->pcm = NULL;
    }
}

/* =============================================================================
 * 播放
 * ========================================================================== */

/* =============================================================================
 * 播放 —— 移植 Mamboe_App 的做法：交给 PulseAudio
 *
 * 为什么要换掉裸 ALSA
 * ------------------
 * 之前用 snd_pcm_writei 直接写硬件，要自己管环形缓冲、预热、start_threshold、
 * XRUN 恢复、period 对齐……每个环节都是一个坑，实测怎么调都卡顿：
 *   - plughw 默认 start_threshold = buffer_size(9600帧/600ms)，攒不满就永远
 *     不启动，表现为"有数据没声音"；
 *   - 改 start_threshold=1 能启动，但 period 是 2400 帧(150ms)，而我们每段
 *     TTS 只先攒 200ms 就开写，写完就等下一句，硬件很快又吃空 → XRUN；
 *   - 在 XRUN / prepare 之间反复，声音就断断续续。
 *
 * 参考项目 Mamboe_App 的做法极简：pa_simple_write() 直接写，什么都不管。
 * 因为 PulseAudio/PipeWire 是**服务端调度**：
 *   - 它自己在后台持续把数据喂给硬件，客户端只管往里塞；
 *   - tlength=150ms 让它维持一个稳定的水位，天然抗抖动；
 *   - 客户端阻塞在 write 上就自动形成背压，不需要我们算缓冲。
 * 同样一份云端 TTS，用 PA 就是丝滑的。
 *
 * 板上跑的是 pipewire-pulse，libpulse-simple.so.0 现成。
 * ========================================================================== */

/* PA 播放流句柄藏在这里，头文件不暴露 PA 类型 */
struct pa_play_s {
    pa_simple *stream;
    int        rate;
    int        channels;
};

static void *play_thread(void *arg)
{
    audio_play_t *pl = (audio_play_t *)arg;
    struct pa_play_s *ps = (struct pa_play_s *)pl->pcm;

    /* chunk 用 20ms，和云端 Opus 帧粒度一致。
     * PA 端会自己合并成 tlength 的块，这里给多大都行，20ms 足够细。*/
    const size_t chunk = (size_t)pl->sample_rate / 50;
    int16_t *buf = malloc(chunk * sizeof(int16_t));
    if (!buf) {
        fprintf(stderr, "[PLAY] 缓冲分配失败\n");
        pl->err = -1;
        return NULL;
    }

    int was_empty = 1;
    struct timespec latency_at = {0};
    unsigned long long latency_samples = 0;
    fprintf(stderr, "[PLAY] 播放线程启动（PulseAudio, chunk=%zuf/%.0fms）\n",
            chunk, chunk * 1000.0 / pl->sample_rate);

    while (pl->running) {
        size_t got = ring_read(&pl->ring, buf, chunk);

        if (got == 0) {
            if (pl->draining) break;
            /* A PA latency snapshot keeps draining after the final write.
             * Advance only UI/playback-position bookkeeping; do not write
             * silence, reconfigure the stream, or change any audio effects. */
            if (latency_at.tv_sec) {
                struct timespec now;
                clock_gettime(CLOCK_MONOTONIC, &now);
                double elapsed = (now.tv_sec - latency_at.tv_sec) +
                                 (now.tv_nsec - latency_at.tv_nsec) * 1e-9;
                unsigned long long drained = elapsed > 0.0 ?
                    (unsigned long long)(elapsed * pl->sample_rate) : 0;
                pl->queued = drained >= latency_samples ? 0 :
                             (long long)(latency_samples - drained);
            }
            was_empty = 1;
            usleep(2000);
            continue;
        }

        /* 每段开头先攒一点点，避免网络抖动直接反映成断音。
         * PA 有 tlength 兜底，这里只需要很短的预热（60ms）即可，
         * 不像裸 ALSA 那样要 200ms。*/
        if (was_empty) {
            const size_t prefill = (size_t)pl->sample_rate * 60 / 1000;
            int waited = 0;
            while (pl->running && ring_avail(&pl->ring) < prefill) {
                if (pl->draining) break;
                usleep(5000);
                waited += 5;
                if (waited > 1500) break;
            }
            was_empty = 0;
        }

        int err = 0;
        if (pa_simple_write(ps->stream, buf, got * sizeof(int16_t), &err) < 0) {
            pl->underruns++;
            fprintf(stderr, "[PLAY] pa_simple_write 失败: code=%d\n", err);
            /* PA 断开就重建流，尽量自愈 */
            pa_simple_free(ps->stream);
            pa_sample_spec ss;
            memset(&ss, 0, sizeof(ss));
            ss.format = PA_SAMPLE_S16LE;
            ss.rate = (uint32_t)pl->sample_rate;
            ss.channels = 1;
            pa_channel_map cm;
            memset(&cm, 0, sizeof(cm));
            cm.channels = 1;
            cm.map[0] = PA_CHANNEL_POSITION_MONO;
            pa_buffer_attr ba;
            memset(&ba, 0, sizeof(ba));
            ba.tlength   = (uint32_t)pl->sample_rate * 2U * 15U / 100U;
            ba.maxlength = ba.tlength * 4U;
            ba.prebuf    = (uint32_t)-1;
            ba.minreq    = (uint32_t)-1;
            ba.fragsize  = (uint32_t)-1;
            int e2 = 0;
            /* 重建时也要回到 AEC sink，否则一旦重连就失去回声消除能力，
             * 表现为"用一会儿突然开始自打断"，很难排查。*/
            ps->stream = pa_simple_new(NULL, "DEMO030", PA_STREAM_PLAYBACK,
                                       "mic-ns-sink", "tts", &ss, &cm, &ba, &e2);
            if (!ps->stream) {
                ps->stream = pa_simple_new(NULL, "DEMO030", PA_STREAM_PLAYBACK,
                                           NULL, "tts", &ss, &cm, &ba, &e2);
            }
            if (!ps->stream) {
                fprintf(stderr, "[PLAY] 重建播放流失败: code=%d\n", e2);
                usleep(50000);
            }
            continue;
        }

        pl->written += (unsigned long long)got;

        /* 用 PA 报告的延迟更新播放位置，供数字人口型对齐。
         * latency 单位是微秒。*/
        int lerr = 0;
        unsigned long long lat_us = (unsigned long long)pa_simple_get_latency(ps->stream, &lerr);
        if (!lerr) {
            unsigned long long queued = lat_us * (unsigned long long)pl->sample_rate / 1000000ULL;
            pl->queued = (long long)queued;
            latency_samples = queued;
            clock_gettime(CLOCK_MONOTONIC, &latency_at);
        }
    }

    if (pl->err == 0 && ps->stream) {
        int derr = 0;
        pa_simple_drain(ps->stream, &derr);
    }
    free(buf);
    fprintf(stderr, "[PLAY] 播放线程退出 underrun=%d\n", pl->underruns);
    return NULL;
}

int audio_play_start(audio_play_t *pl, const char *device,
                     unsigned int sample_rate, size_t ring_ms)
{
    (void)device;       /* PA 用默认 sink，不指定 ALSA 设备 */
    memset(pl, 0, sizeof(*pl));
    snprintf(pl->device, sizeof(pl->device), "%s", "pulse-default");
    pl->sample_rate = sample_rate ? sample_rate : 16000;
    pl->channels = 1;

    size_t cap = (size_t)pl->sample_rate * (ring_ms ? ring_ms : 3000) / 1000;
    if (ring_init(&pl->ring, cap) != 0) return -1;

    struct pa_play_s *ps = calloc(1, sizeof(*ps));
    if (!ps) {
        ring_free(&pl->ring);
        return -1;
    }
    ps->rate = (int)pl->sample_rate;
    ps->channels = 1;

    pa_sample_spec ss;
    memset(&ss, 0, sizeof(ss));
    ss.format   = PA_SAMPLE_S16LE;
    ss.rate     = pl->sample_rate;
    ss.channels = 1;

    pa_channel_map cm;
    memset(&cm, 0, sizeof(cm));
    cm.channels = 1;
    cm.map[0] = PA_CHANNEL_POSITION_MONO;

    /* 和参考项目一致的缓冲参数：
     *   tlength   150ms  → PA 维持的目标水位，稳定性来源
     *   maxlength 600ms  → 上限，客户端狂塞也不会无限涨
     * 其余用 -1 让 PA 自己决定。*/
    pa_buffer_attr ba;
    memset(&ba, 0, sizeof(ba));
    ba.tlength   = (uint32_t)pl->sample_rate * 2U * 15U / 100U;
    ba.maxlength = ba.tlength * 4U;
    ba.prebuf    = (uint32_t)-1;
    ba.minreq    = (uint32_t)-1;
    ba.fragsize  = (uint32_t)-1;

    int err = 0;
    ps->stream = pa_simple_new(NULL, "DEMO030", PA_STREAM_PLAYBACK,
                               NULL, "tts", &ss, &cm, &ba, &err);
    if (!ps->stream) {
        /* 连不上默认 sink 时，退回到显式 sink 名再试一次 */
        ps->stream = pa_simple_new(NULL, "DEMO030", PA_STREAM_PLAYBACK,
                                   "alsa_output.platform-es8389-sound.analog-stereo",
                                   "tts", &ss, &cm, &ba, &err);
    }
    if (!ps->stream) {
        fprintf(stderr, "[PLAY] ✗ 打不开 PulseAudio 播放流 (code=%d)\n", err);
        fprintf(stderr, "[PLAY]   程序若以 root 运行，需要 XDG_RUNTIME_DIR "
                        "指向桌面用户的 runtime 目录\n");
        free(ps);
        ring_free(&pl->ring);
        return -1;
    }

    pl->pcm = ps;
    pl->running = 1;
    pl->draining = 0;
    if (pthread_create(&pl->thread, NULL, play_thread, pl) != 0) {
        pa_simple_free(ps->stream);
        free(ps);
        ring_free(&pl->ring);
        return -1;
    }

    fprintf(stderr, "[PLAY] ✓ PulseAudio 播放流就绪 %uHz/1ch/S16  tlength=150ms\n",
            pl->sample_rate);
    return 0;
}

void audio_play_stop(audio_play_t *pl)
{
    if (!pl) return;
    if (!pl->running && !pl->pcm) return;

    /* 让线程把 ring 里的数据放完，避免最后一个字被截断 */
    pl->draining = 1;
    pl->running = 0;
    if (pl->thread) pthread_join(pl->thread, NULL);
    pl->thread = 0;

    struct pa_play_s *ps = (struct pa_play_s *)pl->pcm;
    if (ps) {
        if (ps->stream) pa_simple_free(ps->stream);
        free(ps);
        pl->pcm = NULL;
    }
    ring_free(&pl->ring);
    pl->running = 0;
    pl->draining = 0;
}

size_t audio_play_write(audio_play_t *pl, const int16_t *pcm, size_t n)
{
    if (!pl || !pcm || n == 0) return 0;
    /* 阻塞写进环形缓冲；满了就等播放线程取走。
     * 背压来自 PA 的 pa_simple_write 阻塞，链路自然限速。*/
    size_t total = 0;
    while (total < n && pl->running) {
        size_t w = ring_write(&pl->ring, pcm + total, n - total);
        total += w;
        if (w == 0) usleep(3000);
    }
    return total;
}

void audio_play_flush(audio_play_t *pl)
{
    if (!pl) return;
    ring_clear(&pl->ring);
    struct pa_play_s *ps = (struct pa_play_s *)pl->pcm;
    if (ps && ps->stream) {
        int err = 0;
        pa_simple_flush(ps->stream, &err);
    }
}

size_t audio_play_pending(audio_play_t *pl)
{
    return ring_avail(&pl->ring);
}

unsigned long long audio_play_written(const audio_play_t *pl)
{
    return pl ? pl->written : 0;
}

unsigned long long audio_play_stamp(audio_play_t *pl)
{
    if (!pl) return 0;
    return pl->written + (unsigned long long)ring_avail(&pl->ring);
}

unsigned long long audio_play_position(audio_play_t *pl)
{
    if (!pl) return 0;
    unsigned long long w = pl->written;
    long long q = pl->queued;
    if (q < 0) q = 0;
    if ((unsigned long long)q > w) return 0;
    return w - (unsigned long long)q;
}
