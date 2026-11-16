/* =============================================================================
 * cloud_chat.h —— 移远云端对话会话封装
 *
 * 把 qthsdk 的 AI Chat 包装成「喂 PCM 进来、吐文本和 PCM 出去」的简单接口，
 * 上层不用关心 WebSocket / Opus / DMP 注册这些细节。
 *
 * ## 云端负责什么
 *
 *   上行：16k PCM  → Opus 编码 → WebSocket → 云端
 *   云端：VAD 切句 → ASR（volcengine 流式）→ LLM → TTS（流式）
 *   下行：WebSocket → Opus 解码 → 16k PCM
 *
 * 也就是说 **VAD / ASR / LLM / TTS 全在云端**，本地只做音频收发，
 * 这是延迟最低的方案（自建 Python 方案光是 edge-tts 就有 2.2s 固定开销）。
 * ========================================================================== */

#ifndef DEMO030_CLOUD_CHAT_H
#define DEMO030_CLOUD_CHAT_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ---- 会话状态 ---- */
typedef enum {
    CC_STATE_IDLE = 0,      /* 未连接 */
    CC_STATE_CONNECTING,    /* 正在建链 / DMP 注册 */
    CC_STATE_READY,         /* 会话就绪，可以发音频 */
    CC_STATE_USER_SPEAK,    /* 云端 VAD 检测到用户说话 */
    CC_STATE_THINKING,      /* 云端在生成回答 */
    CC_STATE_SPEAKING,      /* 云端在推 TTS 音频 */
    CC_STATE_ERROR,
} cc_state_t;

/* ---- 事件回调（都在 SDK 内部线程触发，回调里不要做耗时操作）---- */
typedef struct {
    /* 状态变化 */
    void (*on_state)(cc_state_t st, void *user);
    /* 用户说的话（增量；done=1 表示这句最终结果）*/
    void (*on_asr)(const char *text, int is_delta, void *user);
    /* 回答文本（增量；done=1 表示全文结束）*/
    void (*on_reply)(const char *text, int is_delta, void *user);
    /* 云端下发的 TTS 音频（已 Opus 解码成 16k PCM16 单声道）*/
    void (*on_audio)(const int16_t *pcm, size_t samples, void *user);
    /* 错误 */
    void (*on_error)(int code, const char *msg, void *user);
    void *user;
} cc_callbacks_t;

typedef struct cc_s cc_t;   /* 不透明句柄 */

/* ---- 生命周期 ---- */
/* 建链（内部会做 DMP 注册，约 1~3 秒）。成功返回句柄，失败返回 NULL。 */
cc_t *cc_start(const cc_callbacks_t *cb);

void cc_stop(cc_t *cc);

/* ---- 音频 ---- */
/* 喂 16k/单声道/PCM16。内部会切成 10ms(160采样) 的 Opus 帧上云。 */
void cc_feed_pcm(cc_t *cc, const int16_t *pcm, size_t samples);

/* 主动打断当前回答（发 conversation.cancel）*/
void cc_cancel(cc_t *cc);

/* ---- 状态查询 ---- */
cc_state_t cc_state(const cc_t *cc);
int  cc_is_ready(const cc_t *cc);
/* 最近一次识别的用户问题（供 UI 显示）*/
const char *cc_last_question(const cc_t *cc);
/* 最近一次回答（供 UI 显示）*/
const char *cc_last_reply(const cc_t *cc);

/* 统计 */
typedef struct {
    unsigned long audio_tx_bytes;   /* 上行音频字节数 */
    unsigned long audio_rx_bytes;   /* 下行音频字节数 */
    unsigned long frames_tx;
    unsigned long llm_deltas;
    unsigned long asr_deltas;
} cc_stats_t;
void cc_get_stats(const cc_t *cc, cc_stats_t *out);

#ifdef __cplusplus
}
#endif

#endif
