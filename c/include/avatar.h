/*
 * avatar.h — 数字人显示（口型驱动）
 *
 * 设计思路
 * ========
 * 渲染全部交给浏览器：程序在 127.0.0.1:8765 起一个 HTTP 状态桥
 * （见 avatar_web.c），对外提供 /api/state（JSON）和 web/ 下的静态资源；
 * Firefox kiosk 打开该地址，用 three.js + @pixiv/three-vrm 渲染 VRM 模型。
 * 本模块只负责口型计算与状态发布，不依赖 X11、字体或任何本地贴图。
 *
 * 每 20ms 从真实 TTS 音频计算三个近似口型参数：
 *       openness  张嘴程度（音量包络）
 *       wide      扁平度（过零率 → 齿音/前元音）
 *       round     圆唇度（低频能量占比 → 圆唇音）
 * 播放位置用于选择口型帧；音量和频谱不是音素识别，不保证逐字唇形准确。
 * 端到端同步和目标设备帧率必须实测，不能据静态素材或理论算力推断。
 */
#ifndef AVATAR_H
#define AVATAR_H

#include <stdint.h>
#include <stddef.h>

/* 屏幕分辨率（M2 板载 DSI 屏）*/
#define AV_W 1280
#define AV_H 800

/* 口型帧率：20ms 一帧，和音频块对齐 */
#define AV_VISEME_FPS   50
#define AV_VISEME_HOP   320          /* 16000 / 50 */

/* 口型开放档位数（与 Python 预渲染的 layers 对应）*/
#define AV_MOUTH_LEVELS 8
#define AV_MOUTH_SHAPES 3

/* 运行状态 */
typedef enum {
    AV_STATE_IDLE = 0,   /* 待机 */
    AV_STATE_LISTEN,     /* 用户说话 */
    AV_STATE_THINK,      /* 等待回答 */
    AV_STATE_SPEAK,      /* 播音（口型跟着动）*/
} av_state_t;

typedef struct avatar_s avatar_t;

/* 启动显示。成功返回句柄，失败返回 NULL（失败时程序应退化为无界面模式）。
 * 可用环境变量 DEMO030_NO_GUI=1 强制禁用界面。*/
avatar_t *av_start(void);

/* 关闭显示 */
void av_stop(avatar_t *av);

/* 设置状态（决定表情/文案）*/
void av_set_state(avatar_t *av, av_state_t st);

/* 设置要显示的问答文本（传 NULL 表示该项不变；传 "" 表示清空）*/
void av_set_text(avatar_t *av, const char *question, const char *answer);

/* 更新麦克风电平 0..1（给右侧面板的电平条用）*/
void av_set_mic_level(avatar_t *av, float level);

/* =============================================================================
 * 口型驱动 — 由播放线程在**每个音频块**调用
 * ========================================================================== */

/* 喂入一段即将播放的 PCM，内部按 20ms 分帧算口型参数并入队。
 *
 * base_stamp 是这段 PCM 在播放时间轴上的**起始采样位置**，
 * 由 audio_play_stamp() 取得（= 已写入 ALSA 的量 + 还在 ring 里排队的量）。
 * 必须传这个值，不能自己累加 —— 否则一旦有打断/清空就会错位。*/
void av_feed_pcm(avatar_t *av, const int16_t *pcm, size_t samples,
                 unsigned long long base_stamp);

/* 通知"这段音频播完了"，口型回到闭嘴 */
void av_audio_done(avatar_t *av);

/* 清空口型队列（用户打断时调用）*/
void av_flush(avatar_t *av);

/* 内部：渲染线程循环（main 里不用调，av_start 已经起线程）*/
void av_tick(avatar_t *av);

/* =============================================================================
 * 播放进度来源
 *
 * 口型要跟「真正从喇叭出来的声音」对齐，而这个位置只有播放模块知道。
 * 与其让 avatar 依赖 audio_play_t，不如注册一个回调：main 启动时把
 * 「怎么取播放位置」告诉 avatar，两个模块就解耦了。
 * ========================================================================== */
typedef unsigned long long (*av_posfn_t)(void *user);

/* 注册播放位置来源。传 NULL 则位置恒为 0（口型会一直闭嘴，仅用于自测）。*/
void av_set_position_source(av_posfn_t fn, void *user);

#endif /* AVATAR_H */
