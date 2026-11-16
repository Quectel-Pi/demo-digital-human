# =============================================================================
# DEMO-030 语音对话 + 3D 数字人（纯 C / 云端一体化）
#
# 用法：
#   make            编译（链接 libs/libqthsdk.so，不需要云端 SDK 源码）
#   make deploy     编译 + 推到板子
#   make clean
#
# 板上运行：
#   ./start_bg.sh   启动（各模块依次起：喇叭 → 界面 → 云端 → 麦克风）
#
# 界面是浏览器渲染的 VRM 模型，不使用 X11 / FreeType：
# 程序在 127.0.0.1:8765 提供状态桥与静态资源，Firefox kiosk 打开即可。
# =============================================================================

# ---- 交叉编译工具链 ----
# 必须用 m2-debian-rootfs-toolchain（gcc 14 / glibc 2.38），
# 用系统自带的 aarch64-linux-gnu-gcc（gcc 11 / glibc 2.35）会因为
# libpulse 依赖 GLIBC_2.38 符号而链接失败。
TC      ?= $(if $(strip $(M2_TOOLCHAIN)),$(M2_TOOLCHAIN),$(abspath ../../../toolchains/m2-debian-rootfs-toolchain))
CC      := $(TC)/bin/aarch64-linux-gnu-gcc
STRIP   := $(TC)/bin/aarch64-linux-gnu-strip

TARGET  := demo030_voice
BOARD_DIR := /userdata/demo030
BOARD_BIN := $(BOARD_DIR)/bin/$(TARGET)

# ---- 目录 ----
SRC_DIR     := src
INC_DIR     := include
QTH_INC     := $(INC_DIR)/qthsdk
LIBS        := libs

# ---- 移远云端 SDK ----
# 以**预编译共享库**形式使用，不需要源码：
#   libs/libqthsdk.so   已包含 qthsdk + mbedtls 的全部实现
#   include/qthsdk/     应用编译所需的头文件
QTH_SO      := $(LIBS)/libqthsdk.so

# ---- 编译选项 ----
QTH_INCS = $(shell find $(QTH_INC) -type d 2>/dev/null | sed 's/^/-I/')

CFLAGS   = -std=gnu99 -O2 -Wall -Wextra -Wno-unused-parameter \
           -I$(INC_DIR) $(QTH_INCS) -D_GNU_SOURCE \
           -DDEMO030_ROOT=\"$(BOARD_DIR)\"
LDFLAGS := -lpthread -lm -lrt

# 播放走 PulseAudio（pipewire-pulse），移植自参考项目 Mamboe_App。
# 裸 ALSA 直写需要自己管 start_threshold / XRUN / period 对齐，
# 实测怎么调都卡顿；PA 由服务端调度，天然丝滑。
PA_LIBS  := $(LIBS)/libpulse-simple.so.0 $(LIBS)/libpulse.so.0

# ---- 本地源文件 ----
LOCAL_SRCS := $(wildcard $(SRC_DIR)/*.c)

LOCAL_OBJS := $(patsubst $(SRC_DIR)/%.c,build/%.o,$(LOCAL_SRCS))

# =============================================================================
.PHONY: all deploy run clean check-deps

# 对象文件与链接必须顺序执行，避免并行时的竞争。
.NOTPARALLEL:

all: check-deps $(TARGET)

$(TARGET): $(LOCAL_OBJS) $(QTH_SO)
	@echo "== 链接 =="
	@$(CC) $(LOCAL_OBJS) -Wl,-rpath,'$$ORIGIN/../libs' \
	      $(QTH_SO) $(LIBS)/libopus.so.0 $(PA_LIBS) $(LDFLAGS) -o $@
	@$(STRIP) $@ 2>/dev/null || true
	@echo "✓ 生成 $@"
	@ls -lh $@

build/%.o: $(SRC_DIR)/%.c
	@mkdir -p build
	@echo "  CC $<"
	@$(CC) $(CFLAGS) -c $< -o $@

# ---- 依赖检查 ----
check-deps:
	@[ -x $(CC) ] || { echo "✗ 找不到工具链: $(CC)"; \
	  echo "  用 make TC=/path/to/toolchain 指定"; exit 1; }
	@[ -f $(QTH_SO) ] || { echo "✗ 缺 $(QTH_SO)"; \
	  echo "  这是预编译的云端 SDK，应由仓库自带；缺失请联系维护者"; exit 1; }
	@[ -d $(QTH_INC) ] || { echo "✗ 缺 $(QTH_INC)/"; \
	  echo "  应用编译需要 qthsdk 头文件"; exit 1; }
	@[ -f $(LIBS)/libopus.so.0 ] || { echo "✗ 缺 $(LIBS)/libopus.so.0"; \
	  echo "  执行: adb pull /usr/lib/aarch64-linux-gnu/libopus.so.0 $(LIBS)/"; exit 1; }
	@[ -f $(LIBS)/libpulse-simple.so.0 ] || { echo "✗ 缺 $(LIBS)/libpulse-simple.so.0"; \
	  echo "  执行: adb pull /usr/lib/aarch64-linux-gnu/libpulse-simple.so.0 $(LIBS)/"; exit 1; }
	@[ -f $(LIBS)/libpulse.so.0 ] || { echo "✗ 缺 $(LIBS)/libpulse.so.0"; \
	  echo "  执行: adb pull /usr/lib/aarch64-linux-gnu/libpulse.so.0 $(LIBS)/"; exit 1; }

# ---- 部署 ----
deploy: all
	@adb get-state >/dev/null 2>&1 || { echo "✗ 无 adb 设备"; exit 1; }
	@echo "== 部署到板子 =="
	@adb shell "mkdir -p $(BOARD_DIR)/bin $(BOARD_DIR)/web $(BOARD_DIR)/libs"
	@adb push $(TARGET) $(BOARD_BIN) >/dev/null
	@adb shell "chmod +x $(BOARD_BIN)"
	@echo "  ✓ $(BOARD_BIN)"
	@# 云端 SDK 是独立共享库，必须和可执行文件一起推，否则运行时找不到。
	@adb push $(QTH_SO) $(BOARD_DIR)/libs/ >/dev/null
	@echo "  ✓ $(BOARD_DIR)/libs/libqthsdk.so"
	@# 3D 界面全靠 web/：index.html + app.js + style.css + model.vrm。
	@# node_modules 是 three.js 与 three-vrm 的本地副本（importmap 直接指过去），
	@# 板上没网也不需要 npm，必须一起推。
	@[ -d web ] || { echo "  ✗ 缺 web/ 目录"; exit 1; }
	@adb shell "rm -rf $(BOARD_DIR)/web/node_modules"
	@adb push web/. $(BOARD_DIR)/web/ >/dev/null
	@echo "  ✓ $(BOARD_DIR)/web/ (index.html app.js style.css model.vrm node_modules)"
	@# 启动脚本：板上唯一入口，负责启动程序 + 拉起 Firefox kiosk。
	@adb push start_bg.sh $(BOARD_DIR)/ >/dev/null
	@adb shell "chmod +x $(BOARD_DIR)/start_bg.sh"
	@echo "  ✓ $(BOARD_DIR)/start_bg.sh"
	@echo ""
	@echo "板上运行:"
	@echo "  su q -c '$(BOARD_DIR)/start_bg.sh'"

run: deploy
	@adb shell "cd $(BOARD_DIR) && ./bin/$(TARGET)"

clean:
	@rm -rf build $(TARGET)
	@echo "✓ 已清理（保留 libs 和 include/qthsdk）"
