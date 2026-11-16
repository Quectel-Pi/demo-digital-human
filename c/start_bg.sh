#!/bin/sh
# =============================================================================
# DEMO-030 启动脚本（板上：/userdata/demo030/start_bg.sh）
#
# 一次把两件事都拉起来：
#   1) demo030_voice —— 语音链路 + 127.0.0.1:8765 状态桥
#   2) Firefox kiosk —— 打开状态桥页面，用 WebGL 渲染 3D 数字人
#
# 为什么需要 setsid
# ------------------
# 直接 `adb shell 'nohup ./bin/demo030_voice &'` 是不可靠的：adb 会话结束时
# 会向整个进程组发信号，日志里表现为 "Broken pipe" 然后进程消失。
# 必须用 setsid 让程序进入**独立会话**，才能真正脱离 adb shell 存活。
#
# 用法（必须以桌面用户 q 运行：su q -c ./start_bg.sh）：
#   start_bg.sh            启动（真实麦克风 + 3D 数字人界面）
#   start_bg.sh stop       停止
#   start_bg.sh status     查看状态
#   start_bg.sh log        跟踪日志
# =============================================================================
set -e
DIR=$(cd "$(dirname "$0")" && pwd)
BIN="$DIR/bin/demo030_voice"
WEB="$DIR/web"
# 日志放在程序目录下，属主是运行用户，避免 /userdata/live.log 被 root
# 创建后 q 用户写不进去（Permission denied）。
LOG="$DIR/live.log"
FFLOG="$DIR/firefox.log"
FFPROFILE="$DIR/firefox-profile"

PA_UID=1001
PA_USER=q
export XDG_RUNTIME_DIR=/run/user/$PA_UID
export DISPLAY=:0

# ⚠️ Xauthority 文件名后面那串随机后缀每次登录都会变（.mutter-Xwaylandauth.XXXXXX），
# 不能写死。写死的话 Firefox 连不上 Xwayland → 窗口根本不出来。
# 这里自动挑最新的那个。
XAUTH=""
for f in /run/user/$PA_UID/.mutter-Xwaylandauth.*; do
    [ -f "$f" ] || continue
    XAUTH="$f"          # glob 按名字排序，最后一个即是最近的
done
if [ -z "$XAUTH" ]; then
    for f in /run/user/$PA_UID/.Xauthority "$HOME/.Xauthority"; do
        [ -f "$f" ] && { XAUTH="$f"; break; }
    done
fi
[ -n "$XAUTH" ] && export XAUTHORITY="$XAUTH"
[ -n "${XAUTHORITY:-}" ] || echo "⚠ 未找到 Xwayland 授权文件，界面可能不显示"

case "$1" in
stop)
    for p in $(pgrep -f "[d]emo030_voice" 2>/dev/null); do
        kill "$p" 2>/dev/null || true
    done
    for p in $(pgrep -f "[f]irefox.*8765" 2>/dev/null); do
        kill "$p" 2>/dev/null || true
    done
    sleep 1
    for p in $(pgrep -f "[d]emo030_voice" 2>/dev/null); do
        kill -9 "$p" 2>/dev/null || true
    done
    echo "已停止"
    exit 0
    ;;
status)
    n=$(pgrep -f "[d]emo030_voice" 2>/dev/null | wc -l)
    b=$(pgrep -f "[f]irefox.*8765" 2>/dev/null | wc -l)
    echo "语音进程: $n   浏览器进程: $b"
    if [ "$n" -gt 0 ]; then
        grep -E "^\[状态\]|^\[延迟\]|^\[AVWEB\]" "$LOG" 2>/dev/null | tail -5
    fi
    exit 0
    ;;
log)
    tail -f "$LOG"
    exit 0
    ;;
esac

[ -x "$BIN" ] || { echo "找不到 $BIN"; exit 1; }

# ---- 停掉旧实例 ----
for p in $(pgrep -f "[d]emo030_voice" 2>/dev/null); do
    kill "$p" 2>/dev/null || true
done
sleep 1

# ---- ALSA 混音器 ----
# spk switch = AW8010 功放使能（GPIO4_B2），默认 off，不开完全没声音
amixer -c 0 sset 'spk switch' on    >/dev/null 2>&1 || true
amixer -c 0 sset 'hp switch'  off   >/dev/null 2>&1 || true
amixer -c 0 sset 'Main Mic'    on   >/dev/null 2>&1 || true
amixer -c 0 sset 'Headset Mic' off  >/dev/null 2>&1 || true
amixer -c 0 sset 'ADCL PGA'    11   >/dev/null 2>&1 || true

# ---- 喇叭数字增益 ----
# ⚠️ DACL/DACR 是 ES8389 的 DAC 数字音量，默认 179 = 70% = -6dB。
# 实测该控件每步 0.5dB：179=-6dB、191=0dB、203=+6dB、255=+32dB。
# ⚠️ 不要设 255！那是 +32dB，数字域直接削顶，声音又破又刺。
# 这里用 203（+6dB），比默认提升 12dB，响度明显但不失真。
amixer -c 0 sset 'DACL' 203 >/dev/null 2>&1 || true
amixer -c 0 sset 'DACR' 203 >/dev/null 2>&1 || true

# ---- 把默认设备指向 PipeWire 的 echo-cancel 节点 ----
# ⚠️ 不能靠 pa_simple_new 的 dev 参数：这块板的 PipeWire 会忽略它，
# 流永远落到默认设备上。只有 set-default-* 才真正生效。
# 不设的话播放走裸喇叭、录音走裸麦，两者不在同一个 echo-cancel 环里，
# 喇叭放的每句话都被重新录进去 → 云端误判插话 → 自打断。
#
# ⚠️⚠️ 这里所有 pactl 必须以桌面用户身份执行！本脚本由 root 跑，
# root 直连 PulseAudio 会得到 "Connection refused"（code=6），
# 表现为命令行什么都"成功"了但设置完全没生效。
#
# 用函数而不是 `PA="su q -c"; $PA "..."` 这种写法：后者 $PA 没加引号，
# 会被拆成多个参数，su 只拿到第一个词，命令根本没执行，还会卡住。
pa() {
    su "$PA_USER" -c "XDG_RUNTIME_DIR=/run/user/$PA_UID $*"
}

pa pactl set-default-sink mic-ns-sink >/dev/null 2>&1 \
    && echo "  ✓ 播放 → mic-ns-sink（回声消除）" \
    || echo "  ⚠ 无 mic-ns-sink，会自打断"
pa pactl set-default-source mic-ns >/dev/null 2>&1 \
    && echo "  ✓ 录音 ← mic-ns（回声消除）" \
    || echo "  ⚠ 无 mic-ns，会自打断"

# ---- 音量 ----
# ⚠️ 声音实际经过两级：应用 → mic-ns-sink(回环桩) → alsa_output.es8389(硬件)。
# mic-ns-sink 默认 100%，但 alsa_output 那一级经常只有 40%（-23.88dB），
# 这是「喇叭声音很小」的直接原因。两级都要拉满。
pa pactl set-sink-mute mic-ns-sink 0 >/dev/null 2>&1 || true
pa pactl set-sink-volume mic-ns-sink device=100% >/dev/null 2>&1 || true
pa pactl set-sink-volume mic-ns-sink 100% >/dev/null 2>&1 || true
for s in $(pa pactl list short sinks 2>/dev/null | awk '$2 ~ /^alsa_output/ {print $2}'); do
    pa pactl set-sink-volume "$s" 100% >/dev/null 2>&1 \
        && echo "  ✓ 硬件输出 $s → 100%"
done

# ---- setsid 脱离会话启动 ----
# ⚠️ 必须先 cd 到工作目录再启动。
#
# qthsdk 会在**当前工作目录**读写 qth_config.bin，目录不可写时它会陷入
# 死循环疯狂重试，日志里全是：
#     <E/LQTH_CFGSV> Quos_configWriteCb open fail, filename:qth_config.bin
# 实测几秒钟就能把日志刷到 15MB、几百万行，把真正有用的信息全淹掉。
# 之前用 `cd /userdata/demo030 && adb shell ...` 时没暴露，是因为 adb 已经
# 把 cwd 设好了；改成 setsid 后 cwd 变成 /，才踩到这个坑。
touch "$LOG" 2>/dev/null || true
cd "$DIR"

# ---- 以桌面用户 q 身份启动 ----
# ⚠️ 不能用 root 直接启动二进制：PulseAudio 走 native 协议只认「同一个桌面
# 用户」的连接，root 连会拿到 code=6（连接被拒绝），表现为「[PLAY] ✗ 打不开
# PulseAudio 播放流」。板上桌面用户是 uid=1001(q)，同时是声卡与 X 的属主。
#
# ⚠️⚠️ 这里**不要**再套一层 su！
# 本脚本本身就必须以 q 身份启动（调用方式：su q -c start_bg.sh）。如果这里
# 再写 `setsid su q -c ...`，就是 su 套 su —— 内层 su 在后台没有终端可读密码，
# 直接 "su: Authentication failure"，日志里只有一行 Password: ，看起来像
# 「程序启动就退出」但完全看不出原因。这个坑排查了很久。
#
# 正确做法：判断当前 uid，非 q 才提权；已经是 q 就直接 setsid 起进程。
#
# DEMO030_WEB_ROOT 必须显式传：程序靠它判断走 3D 界面，
# 不传的话 av_start() 直接返回 NULL，界面完全没有。
if [ "$(id -u)" = "$PA_UID" ]; then
    setsid env DEMO030_WEB_ROOT="$WEB" "$BIN" > "$LOG" 2>&1 < /dev/null &
else
    setsid su "$PA_USER" -c "cd '$DIR' && \
        XDG_RUNTIME_DIR=/run/user/$PA_UID DISPLAY=$DISPLAY XAUTHORITY=$XAUTHORITY \
        DEMO030_WEB_ROOT='$WEB' '$BIN'" > "$LOG" 2>&1 < /dev/null &
fi
sleep 4

# ⚠️ pgrep -c 在无匹配时输出 0 但退出码非 0，配合 `|| echo 0` 会变成两行
# "0\n0"，导致 [: Illegal number。这里改用 wc -l 计数。
n=$(pgrep -f "[d]emo030_voice" 2>/dev/null | wc -l)
if [ "$n" -gt 0 ]; then
    echo "✓ 已启动（$n 个进程），日志 $LOG"
    grep -E "^\[PLAY\] ✓|^\[CAP\] ✓|^\[AVWEB\] ✓|云端会话就绪|开始对话" "$LOG" 2>/dev/null | head -6
else
    echo "✗ 启动失败，日志如下："
    tail -20 "$LOG" 2>/dev/null
    exit 1
fi

# ---- 拉起 Firefox kiosk 显示 3D 界面 ----
# 程序自己只提供 HTTP 状态桥，不画任何像素；真正的 3D 渲染在浏览器里。
# 单独一个 profile 目录：避免污染用户默认配置，也方便多实例隔离。
if [ ! -f "$WEB/index.html" ]; then
    echo "⚠ 缺 $WEB/index.html，跳过浏览器启动"
    exit 0
fi
mkdir -p "$FFPROFILE"
touch "$FFLOG" 2>/dev/null || true
# 已经在跑就不重复开（否则会叠出多个全屏窗口）
if [ "$(pgrep -f "[f]irefox.*8765" 2>/dev/null | wc -l)" -gt 0 ]; then
    echo "✓ 浏览器已在运行"
    exit 0
fi
setsid env DISPLAY="$DISPLAY" XAUTHORITY="$XAUTHORITY" \
    MOZ_ENABLE_WAYLAND=1 firefox --no-remote --profile "$FFPROFILE" \
    --kiosk "http://127.0.0.1:8765/" > "$FFLOG" 2>&1 < /dev/null &
sleep 3
b=$(pgrep -f "[f]irefox.*8765" 2>/dev/null | wc -l)
if [ "$b" -gt 0 ]; then
    echo "✓ 3D 界面已显示（浏览器 $b 个进程）"
else
    echo "⚠ 浏览器未起来，看 $FFLOG"
    tail -10 "$FFLOG" 2>/dev/null
fi

exit 0
