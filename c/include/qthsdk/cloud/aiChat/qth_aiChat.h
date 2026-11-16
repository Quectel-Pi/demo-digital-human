#ifndef __QTH_AICHAT_H__
#define __QTH_AICHAT_H__

#include "qhal_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ===========================================================================
 * Enumerations
 * =========================================================================*/

/**
 * @brief 工作模式
 */
typedef enum {
    QTH_AI_CHAT_MODE_STT_ONLY    = 0, /**< 仅语音识别 */
    QTH_AI_CHAT_MODE_STT_LLM     = 1, /**< 语音识别 + 大模型 */
    QTH_AI_CHAT_MODE_STT_LLM_TTS = 2, /**< 语音识别 + 大模型 + 语音合成 */
} Qth_aiChatMode_e;

/**
 * @brief 流传输协议类型
 */
typedef enum {
    QTH_AI_CHAT_PROTOCOL_REALTIME = 0, /**< 实时协议 */
    QTH_AI_CHAT_PROTOCOL_LOCAL    = 1, /**< 本地协议 */
} Qth_aiChatProtocol_e;

/**
 * @brief 音频编码格式
 */
typedef enum {
    QTH_AI_CHAT_AUDIO_FMT_PCM16    = 0, /**< 16-bit PCM (pcm16) */
    QTH_AI_CHAT_AUDIO_FMT_OPUS     = 1, /**< Opus 编码 (opus) */
    QTH_AI_CHAT_AUDIO_FMT_G711_ALAW = 2, /**< G.711 A-law 8-bit 8kHz (g711_alaw) */
} Qth_aiChatAudioFormat_e;

/**
 * @brief VAD 触发模式
 */
typedef enum {
    QTH_AI_CHAT_VAD_SERVER_VAD  = 0, /**< 服务端自动 VAD 检测 */
    QTH_AI_CHAT_VAD_MANUAL      = 1, /**< 客户端手动提交（发送时使用 manual，服务端回应时为 push_to_talk） */
} Qth_aiChatVadType_e;

/**
 * @brief AI Chat 事件 ID
 */
typedef enum {
    QTH_AI_CHAT_EVENT_CONNECTED             = 0,  /**< WebSocket 连接已建立 */
    QTH_AI_CHAT_EVENT_DISCONNECTED,               /**< WebSocket 连接已断开 */
    QTH_AI_CHAT_EVENT_SESSION_CREATED,            /**< 会话已建立（session.created） */
    QTH_AI_CHAT_EVENT_SESSION_COMPLETED,          /**< 会话已结束（session.completed） */
    QTH_AI_CHAT_EVENT_SESSION_CONFIG_UPDATED,     /**< 会话配置已更新（session.config.updated） */
    QTH_AI_CHAT_EVENT_VAD_STARTED,                /**< 用户开始说话（session.vad.started） */
    QTH_AI_CHAT_EVENT_VAD_STOPPED,                /**< 用户停止说话（session.vad.stopped） */
    QTH_AI_CHAT_EVENT_VAD_COMPLETED,              /**< VAD 缓冲区提交完成（session.vad.completed） */
    QTH_AI_CHAT_EVENT_TRANSCRIPT_DELTA,           /**< ASR 增量识别结果 */
    QTH_AI_CHAT_EVENT_TRANSCRIPT_COMPLETED,       /**< ASR 识别最终完成 */
    QTH_AI_CHAT_EVENT_CONVERSATION_CREATED,       /**< 对话已建立（session.conversation.created） */
    QTH_AI_CHAT_EVENT_CONVERSATION_COMPLETED,     /**< 对话已完成（session.conversation.completed） */
    QTH_AI_CHAT_EVENT_CONVERSATION_CANCELLED,     /**< 对话已中断（session.conversation.cancelled） */
    QTH_AI_CHAT_EVENT_MESSAGE_DELTA,              /**< LLM 增量文本输出 */
    QTH_AI_CHAT_EVENT_MESSAGE_COMPLETED,          /**< LLM 输出已完成 */
    QTH_AI_CHAT_EVENT_AUDIO_OUTPUT_STARTED,       /**< TTS 首音开始 */
    QTH_AI_CHAT_EVENT_AUDIO_OUTPUT_DATA,          /**< TTS 音频数据块（二进制帧） */
    QTH_AI_CHAT_EVENT_AUDIO_OUTPUT_COMPLETED,     /**< TTS 输出已完成 */
    QTH_AI_CHAT_EVENT_ERROR,                      /**< 错误事件 */
    QTH_AI_CHAT_EVENT_MAX,
} Qth_aiChatEventId_e;

/* ===========================================================================
 * Profile structures (returned by server in session.created / config.updated)
 * =========================================================================*/

/**
 * @brief ASR 服务配置（asrProfile）
 */
typedef struct {
    quint8_t supplier[32];     /**< 供应商，如 "aliyun" */
    quint8_t service_type[32]; /**< 服务类型，如 "stream" */
    quint8_t model_id[64];     /**< 模型 ID */
} Qth_aiChatAsrProfile_t;

/**
 * @brief TTS 服务配置（ttsProfile）
 */
typedef struct {
    quint8_t supplier[32];     /**< 供应商，如 "tencent" */
    quint8_t service_type[32]; /**< 服务类型，如 "stream" */
    quint8_t voice[32];        /**< 音色 ID */
} Qth_aiChatTtsProfile_t;

/**
 * @brief LLM 服务配置（llmProfile）
 */
typedef struct {
    quint8_t supplier[32];         /**< 供应商，如 "aiot" */
    quint8_t service_type[32];     /**< 服务类型，如 "aiagent" */
    quint8_t conversation_id[64];  /**< 会话上下文 ID */
} Qth_aiChatLlmProfile_t;

/* ===========================================================================
 * Event data structures
 * =========================================================================*/

/**
 * @brief session.created 事件数据
 *
 * 服务端在握手完成后返回协商结果，设备应以这里的值覆盖初始配置。
 */
typedef struct {
    quint8_t  sid[64];       /**< session ID */
    quint8_t  msgId[64];     /**< 消息 ID */
    quint64_t timestamp;     /**< 服务端时间戳 */
    quint8_t  mode[32];      /**< 工作模式字符串 (STT_ONLY/STT_LLM/STT_LLM_TTS) */
    Qth_aiChatAudioFormat_e input_audio_format;  /**< 服务端协商的输入音频格式 */
    Qth_aiChatAudioFormat_e output_audio_format; /**< 服务端协商的输出音频格式 */
    quint32_t audio_push_interval_ms;              /**< 服务端协商的音频推送间隔（ms） */
    quint32_t audio_size_per_pkg;                  /**< 服务端协商的每包音频字节数 */
    qbool denoise_enabled;                     /**< 降噪状态：TRUE=开 FALSE=关 */
    qbool aec_enabled;                         /**< 回音消除状态：TRUE=开 FALSE=关 */
    Qth_aiChatAsrProfile_t asr_profile;          /**< ASR 服务配置 */
    Qth_aiChatTtsProfile_t tts_profile;          /**< TTS 服务配置 */
    Qth_aiChatLlmProfile_t llm_profile;          /**< LLM 服务配置 */
} Qth_aiChatSessionCreatedData_t;

/**
 * @brief session.config.updated 事件数据
 *
 * 字段若在服务端响应中未出现，则保持默认值；布尔字段是否出现由对应
 * 的 *_valid 标志指示，整数型字段仍以 0 表示未返回。
 */
typedef struct {
    quint8_t sid[64];   /**< session ID */
    quint8_t msgId[64]; /**< 消息 ID */
    quint8_t mode[32];  /**< 工作模式字符串，空表示未变 */
    Qth_aiChatAudioFormat_e input_audio_format;  /**< 协商的输入音频格式 */
    Qth_aiChatAudioFormat_e output_audio_format; /**< 协商的输出音频格式 */
    qbool  denoise_enabled;        /**< 降噪状态：TRUE=开 FALSE=关 */
    qbool  aec_enabled;            /**< 回音消除状态：TRUE=开 FALSE=关 */
    quint32_t  audio_push_interval_ms; /**< 服务端确认的推送间隔（ms），0=未变 */
    quint32_t  audio_size_per_pkg;     /**< 服务端确认的每包字节数，0=未变 */
    quint32_t  idle_timeout_ms;        /**< 空闲超时（ms），0=未含此字段 */
    /* turnDetection（仅当 turn_detection_valid=TRUE 时有效） */
    qbool                      turn_detection_valid;               /**< TRUE=响应包含 turnDetection 字段 */
    Qth_aiChatVadType_e turn_detection_type;                /**< VAD 模式 */
    double               turn_detection_threshold;           /**< VAD 阈值，0=未设置 */
    quint32_t                  turn_detection_prefix_padding_ms;   /**< 前置填充（ms），0=未设置 */
    quint32_t                  turn_detection_silence_duration_ms; /**< 静音判断（ms），0=未设置 */
    quint32_t                  turn_detection_min_speech_duration_ms; /**< 最短说话时长（ms），0=未设置 */
    Qth_aiChatAsrProfile_t asr_profile;                     /**< ASR 服务配置 */
    Qth_aiChatTtsProfile_t tts_profile;                     /**< TTS 服务配置 */
    Qth_aiChatLlmProfile_t llm_profile;                     /**< LLM 服务配置 */
    qbool denoise_valid;                                    /**< TRUE=响应包含 denoiseEnabled 字段 */
    qbool aec_valid;                                        /**< TRUE=响应包含 aecEnabled 字段 */
} Qth_aiChatConfigUpdatedData_t;

/**
 * @brief VAD 类事件数据（vad.started / vad.stopped / vad.completed）
 */
typedef struct {
    quint8_t sid[64];   /**< session ID */
    quint8_t cid[64];   /**< conversation ID（若已建立对话） */
    quint8_t vadId[64]; /**< VAD 生命周期 ID */
    quint8_t msgId[64]; /**< 消息 ID */
} Qth_aiChatVadData_t;

/**
 * @brief ASR 类事件数据（transcript.delta / transcript.completed）
 *
 * @note content 指针直接指向内部接收缓冲区，仅在回调期间有效，
 *       如需保留须自行 memcpy。
 */
typedef struct {
    const quint8_t *content;     /**< 识别文本起始指针（非 NUL 结尾，长度见 content_len） */
    quint32_t    content_len; /**< 识别文本字节数 */
    quint8_t sid[64];         /**< session ID */
    quint8_t cid[64];         /**< conversation ID */
    quint8_t asrId[64];       /**< ASR 生命周期 ID */
    quint8_t msgId[64];       /**< 消息 ID */
    quint32_t asr_usage;      /**< ASR 用量（秒），0=未含此字段；仅 transcript.completed 有效 */
} Qth_aiChatTranscriptData_t;

/**
 * @brief 对话类事件数据（conversation.created / conversation.completed）
 */
typedef struct {
    quint8_t  sid[64];   /**< session ID */
    quint8_t  cid[64];   /**< conversation ID */
    quint8_t  msgId[64]; /**< 消息 ID */
    quint32_t tts_usage; /**< TTS 用量（秒），0=未含此字段；仅 conversation.completed 有效 */
} Qth_aiChatConversationData_t;

/**
 * @brief conversation.cancelled 事件数据
 */
typedef struct {
    quint8_t  sid[64];   /**< session ID */
    quint8_t  cid[64];   /**< conversation ID */
    quint8_t  msgId[64]; /**< 消息 ID */
    qint32_t code;      /**< 中断原因：1=语音打断  2=主动 cancel  3=手动提交新内容 */
} Qth_aiChatConversationCancelledData_t;

/**
 * @brief LLM 类事件数据（message.delta / message.completed）
 *
 * @note content 指针直接指向内部接收缓冲区，仅在回调期间有效，
 *       如需保留须自行 memcpy。
 */
typedef struct {
    const quint8_t *content;     /**< 文本内容起始指针（非 NUL 结尾，长度见 content_len） */
    quint32_t    content_len; /**< 文本字节数 */
    quint8_t sid[64];         /**< session ID */
    quint8_t cid[64];         /**< conversation ID */
    quint8_t llmId[64];       /**< LLM 生命周期 ID */
    quint8_t msgId[64];       /**< 消息 ID */
} Qth_aiChatMessageData_t;

/**
 * @brief TTS 生命周期事件数据（audio_output.started / audio_output.completed）
 */
typedef struct {
    quint8_t  sid[64];   /**< session ID */
    quint8_t  cid[64];   /**< conversation ID */
    quint8_t  ttsId[64]; /**< TTS 生命周期 ID */
    quint8_t  msgId[64]; /**< 消息 ID */
    quint32_t tts_usage; /**< TTS 用量（秒），0=未含此字段；仅 audio_output.completed 有效 */
} Qth_aiChatAudioOutputData_t;

/**
 * @brief TTS 音频数据块事件（QTH_AI_CHAT_EVENT_AUDIO_OUTPUT_DATA）
 */
typedef struct {
    const void *data; /**< 音频数据指针（仅回调期间有效） */
    quint32_t    len;  /**< 数据长度（字节） */
} Qth_aiChatAudioChunkData_t;

/**
 * @brief 错误事件数据
 */
typedef struct {
    quint8_t  sid[64];   /**< session ID */
    quint8_t  msgId[64]; /**< 消息 ID */
    quint32_t code;      /**< 错误码（数字，如 10001）；HTTP 握手错误时为 0 */
    quint8_t  msg[256];  /**< 错误描述 */
    quint32_t http_status_code; /**< HTTP 握手失败时的状态码（如 401、400）；0 表示非 HTTP 类错误 */
} Qth_aiChatErrorData_t;

/* ===========================================================================
 * Session config structures (for Qth_aiChatUpdateSessionConfig)
 * =========================================================================*/

/**
 * @brief 打断检测配置
 */
typedef struct {
    const quint8_t **keywords; /**< 打断关键词列表（NULL 结尾数组），NULL 表示不设置 */
    const quint8_t  *mode;     /**< "keyword_contains" 或 "keyword_prefix"，NULL 表示不设置 */
} Qth_aiChatInterruptConfig_t;

/**
 * @brief VAD / 转折检测配置
 */
typedef struct {
    Qth_aiChatVadType_e          type;               /**< VAD 模式 */
    double                        threshold;           /**< VAD 阈值（0 表示不设置） */
    qint32_t                      prefix_padding_ms;   /**< 前置填充毫秒数（0 表示不设置） */
    qint32_t                      silence_duration_ms; /**< 静音判断毫秒数（0 表示不设置） */
    qint32_t                      min_speech_duration_ms; /**< 最小语音持续时长毫秒数（0 表示不设置） */
    qbool                         smart_turn_enabled;  /**< 智能断句开关（FALSE 表示不设置） */
    double                        smart_turn_threshold;/**< 智能断句阈值（0 表示不设置） */
    qbool                         energy_gating_enabled; /**< 能量阈值门控开关（FALSE 表示不设置） */
    qint32_t                      min_energy_rms;      /**< 最小能量 RMS 阈值（0 表示不设置） */
    qbool                         zcr_gating_enabled;  /**< 过零率门控开关（FALSE 表示不设置） */
    double                        min_zcr;             /**< 最小过零率阈值（0 表示不设置） */
    double                        max_zcr;             /**< 最大过零率阈值（0 表示不设置） */
    const Qth_aiChatInterruptConfig_t *interrupt;    /**< 打断配置，NULL 表示不设置 */
} Qth_aiChatTurnDetection_t;

/**
 * @brief 会话配置更新参数
 */
typedef struct {
    const Qth_aiChatTurnDetection_t *turn_detection; /**< VAD 配置，NULL 表示不更新 */
    qbool                             denoise_enabled; /**< 降噪开关 */
    qbool                             denoise_set;     /**< TRUE 表示需要更新降噪配置 */
    const quint8_t                   *speech_rate;    /**< 语速："fast"(x1.25), "normal"(x1), "slow"(x0.75)，NULL 表示不设置 */
} Qth_aiChatSessionConfig_t;

/* ===========================================================================
 * Main config & handle
 * =========================================================================*/

/**
 * @brief AI Chat 客户端句柄（不透明指针）
 */
typedef struct Qth_aiChat_s *Qth_aiChatHandle_t;

/**
 * @brief 事件回调函数类型
 *
 * @param handle       产生事件的 AI Chat 句柄
 * @param event_id     事件 ID，参见 Qth_aiChatEventId_e
 * @param event_data   事件附带数据（类型随 event_id 变化，参见各 *Data_t 结构体）
 * @param user_context 初始化配置中传入的用户自定义指针
 */
typedef void (*Qth_aiChatEventHandler_f)(Qth_aiChatHandle_t  handle,
                                           Qth_aiChatEventId_e event_id,
                                           void                *event_data,
                                           void                *user_context);

/**
 * @brief AI Chat 客户端初始化配置
 */
typedef struct {
    /* --- 连接地址 --- */
    const quint8_t *url;           /**< 服务 URL，例如 "wss://uat-ai-chat.quectel.com.cn/ws/v1/chat" */
    /* --- 连接参数 --- */
    Qth_aiChatMode_e        mode;                /**< 工作模式 */
    Qth_aiChatProtocol_e    protocol;            /**< 协议类型 */
    Qth_aiChatAudioFormat_e input_audio_format;  /**< 输入音频格式 */
    Qth_aiChatAudioFormat_e output_audio_format; /**< 输出音频格式 */
    quint32_t                 audio_push_interval; /**< 音频推送间隔（ms），建议 100-1000，0 使用默认 100 */
    quint32_t                audio_size_per_pkg;  /**< 每包音频字节数，建议 800-16000，0 使用默认 3200 */
    qbool                    debug;               /**< 开启服务端调试日志 */

    /* --- Opus 输出参数（仅当 input/output 均为 Opus 时才追加到会话 URL） --- */
    qbool     opus_output_use_cbr;       /**< Opus 输出 CBR 开关 */
    quint32_t opus_output_frame_size_ms; /**< Opus 输出帧长(ms)：10/20/40/60，0 使用默认 20 */

    /* --- 回调 --- */
    Qth_aiChatEventHandler_f event_handler; /**< 事件回调，不可为 NULL */
    void                     *user_context;  /**< 用户自定义上下文，回调时原样返回 */

    /* --- 底层 WebSocket 参数（可选） --- */
    quint32_t     ws_buffer_size;   /**< WebSocket 收发缓冲区大小（字节），0 使用默认 16384 */
    quint32_t    ws_task_stack;    /**< WebSocket 内部任务栈大小（字节），0 使用默认値 */
    quint32_t     ws_task_priority; /**< WebSocket 内部任务优先级，0 使用默认値 */
} Qth_aiChatConfig_t;

/* ===========================================================================
 * API
 * =========================================================================*/

/**
 * @brief 初始化 AI Chat 客户端
 *
 * @note  config 采用浅拷贝（`*config` 的内容被整体复制到内部句柄），
 *        config->url 所指向的字符串会被深拷贝到句柄内部。
 *        调用方在函数返回后即可安全释放 config 及 url 缓冲区。
 * @param config  初始化配置，不可为 NULL
 * @return        成功返回客户端句柄；失败返回 NULL
 */
Qth_aiChatHandle_t Qth_aiChatInit(const Qth_aiChatConfig_t *config);

/**
 * @brief 销毁 AI Chat 客户端，释放所有资源
 *
 * @param handle  客户端句柄
 * @return        成功返回 0；失败返回负值错误码
 */
oprt_ret Qth_aiChatDestroy(Qth_aiChatHandle_t handle);

/**
 * @brief 发起 AI Chat 连接（含鉴权）
 *
 * 异步接口。连接结果通过以下事件通知：
 * - QTH_AI_CHAT_EVENT_CONNECTED：WebSocket 握手成功
 * - QTH_AI_CHAT_EVENT_SESSION_CREATED：会话已建立，可以收发数据
 * - QTH_AI_CHAT_EVENT_ERROR / QTH_AI_CHAT_EVENT_DISCONNECTED：连接失败
 *
 * @param handle  客户端句柄
 * @return        成功发起连接返回 0；参数错误返回负值
 */
oprt_ret Qth_aiChatConnect(Qth_aiChatHandle_t handle);

/**
 * @brief 断开 AI Chat 连接
 *
 * @param handle  客户端句柄
 * @return        成功返回 0；失败返回负值错误码
 */
oprt_ret Qth_aiChatDisconnect(Qth_aiChatHandle_t handle);

/**
 * @brief 查询是否处于已连接状态
 *
 * @param handle  客户端句柄
 * @return        已连接（WebSocket 层）返回 0；否则返回负值错误码
 */
oprt_ret Qth_aiChatIsConnected(Qth_aiChatHandle_t handle);

/**
 * @brief 发送音频流数据
 *
 * 以 WebSocket Binary 帧发送，帧首附 4 字节类型标识 {0x00,0x00,0x00,0x05}。
 *
 * @param handle      客户端句柄
 * @param data        音频数据缓冲区
 * @param len         数据字节数
 * @param timeout_ms  发送超时（毫秒）
 * @return            成功返回 0；失败返回负值错误码
 */
oprt_ret Qth_aiChatSendAudio(Qth_aiChatHandle_t handle,
                               const void         *data,
                               quint32_t            len,
                               quint32_t            timeout_ms);

/**
 * @brief 手动提交音频缓冲区（手动 VAD 模式）
 *
 * 发送 session.vad.complete 事件，通知服务端处理已推送的音频。
 *
 * @param handle  客户端句柄
 * @return        成功返回 0；失败返回负值错误码
 */
oprt_ret Qth_aiChatVadComplete(Qth_aiChatHandle_t handle);

/**
 * @brief 取消当前对话
 *
 * 发送 session.conversation.cancel 事件。
 *
 * @param handle  客户端句柄
 * @return        成功返回 0；失败返回负值错误码
 */
oprt_ret Qth_aiChatCancelConversation(Qth_aiChatHandle_t handle);

/**
 * @brief 清空服务端音频输入缓存
 *
 * 发送 session.audio_input.clear 事件。
 *
 * @param handle  客户端句柄
 * @return        成功返回 0；失败返回负值错误码
 */
oprt_ret Qth_aiChatClearAudioInput(Qth_aiChatHandle_t handle);

/**
 * @brief 更新会话配置
 *
 * 发送 session.config.update 事件，服务端响应 session.config.updated。
 *
 * @param handle  客户端句柄
 * @param cfg     新配置，NULL 字段表示该项不更新
 * @return        成功返回 0；失败返回负值错误码
 */
oprt_ret Qth_aiChatUpdateSessionConfig(Qth_aiChatHandle_t           handle,
                                         const Qth_aiChatSessionConfig_t *cfg);

/**
 * @brief 主动结束当前会话
 *
 * 发送 session.completed 事件，服务端将关闭本次 session。
 *
 * @param handle  客户端句柄
 * @return        成功返回 0；失败返回负值错误码
 */
oprt_ret Qth_aiChatSessionComplete(Qth_aiChatHandle_t handle);

/**
 * @brief 以文本帧直接发送原始 JSON 数据（调试/扩展用）
 *
 * @param handle      客户端句柄
 * @param json        JSON 字符串（无需 NUL 结尾，按 len 截取）
 * @param len         长度（字节）
 * @param timeout_ms  发送超时（毫秒）
 * @return            成功返回 0；失败返回负值错误码
 */
oprt_ret Qth_aiChatSendRawText(Qth_aiChatHandle_t handle,
                                 const quint8_t         *json,
                                 quint32_t            len,
                                 quint32_t            timeout_ms);

/**
 * @brief 发送文本输入给 LLM，触发 LLM -> TTS 流程（text_input.chat）
 *
 * 对应协议事件 text_input.chat。服务端收到后将走 LLM->TTS 路径，
 * 产生与语音对话相同的下行事件序列（conversation.created / message.* / audio_output.*）。
 *
 * @param handle      客户端句柄
 * @param text        UTF-8 文本内容（NUL 结尾）
 * @param timeout_ms  发送超时（毫秒）
 * @return            成功返回 0；失败返回负值错误码
 */
oprt_ret Qth_aiChatTextChat(Qth_aiChatHandle_t handle,
                              const quint8_t         *text,
                              quint32_t            timeout_ms);

/**
 * @brief 发送文本直接生成 TTS 音频（text_input.generate_audio）
 *
 * 对应协议事件 text_input.generate_audio。服务端不经过 LLM，直接将文本
 * 送给 TTS 合成，产生 audio_output.started / DATA / audio_output.completed 事件。
 *
 * @param handle      客户端句柄
 * @param text        UTF-8 文本内容（NUL 结尾）
 * @param timeout_ms  发送超时（毫秒）
 * @return            成功返回 0；失败返回负值错误码
 */
oprt_ret Qth_aiChatTextGenerateAudio(Qth_aiChatHandle_t handle,
                                       const quint8_t         *text,
                                       quint32_t            timeout_ms);

#ifdef __cplusplus
}
#endif

#endif /* __QTH_AICHAT_H__ */
