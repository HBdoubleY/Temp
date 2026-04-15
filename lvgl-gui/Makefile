#
# Makefile
#
COMPILE_PREX = ../toolchain/toolchain-sunxi-musl/toolchain/bin/arm-openwrt-linux-muslgnueabi-



CPP = $(COMPILE_PREX)g++
AR = $(COMPILE_PREX)ar
CC = $(COMPILE_PREX)gcc
XX = $(COMPILE_PREX)g++
NM = $(COMPILE_PREX)nm
CPP = $(COMPILE_PREX)g++
STRIP = $(COMPILE_PREX)strip
SIZE = $(COMPILE_PREX)size
OBJCOPY = $(COMPILE_PREX)objcopy
OBJDUMP = $(COMPILE_PREX)objdump
LD = $(COMPILE_PREX)ld


LVGL_DIR_NAME = lvgl
LVGL_DIR ?= ${shell pwd}
PRJ_DIR := ./ui
#CFLAGS = -Wall -O3 -pipe -g0  -I$(LVGL_DIR)/ -march=armv7-a -mtune=cortex-a7 -mfpu=neon -fno-caller-saves -Wno-unused-result -mfloat-abi=hard  -Wformat -Werror=format-security -fstack-protector -Wl,-z,now -Wl,-z,relro -ffunction-sections -fdata-sections  -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 -D__STDC_CONSTANT_MACROS -D_GNU_SOURCE -D_REENTRANT

CFLAGS = -O3 -g0 -march=armv7-a -mtune=cortex-a7 -mfpu=neon -mfloat-abi=hard -ffunction-sections -fdata-sections -I$(LVGL_DIR)/ -Wall -Wshadow -Wundef -Wmissing-prototypes -Wno-discarded-qualifiers -Wall -Wextra -Wno-unused-function -Wno-error=strict-prototypes -Wpointer-arith -fno-strict-aliasing -Wno-error=cpp -Wuninitialized -Wmaybe-uninitialized -Wno-unused-parameter -Wno-missing-field-initializers -Wtype-limits -Wsizeof-pointer-memaccess -Wno-format-nonliteral -Wno-cast-qual -Wunreachable-code -Wno-switch-default -Wreturn-type -Wmultichar -Wformat-security -Wno-ignored-qualifiers -Wno-error=pedantic -Wno-sign-compare -Wno-error=missing-prototypes -Wdouble-promotion -Wclobbered -Wdeprecated -Wempty-body -Wtype-limits -Wshift-negative-value -Wstack-usage=2048 -Wno-unused-value -Wno-unused-parameter -Wno-missing-field-initializers -Wuninitialized -Wmaybe-uninitialized -Wall -Wextra -Wno-unused-parameter -Wno-missing-field-initializers -Wtype-limits -Wsizeof-pointer-memaccess -Wno-format-nonliteral -Wpointer-arith -Wno-cast-qual -Wmissing-prototypes -Wunreachable-code -Wno-switch-default -Wreturn-type -Wmultichar -Wno-discarded-qualifiers -Wformat-security -Wno-ignored-qualifiers -Wno-sign-compare -Wunused-variable -Wunused-variable

#  -O1 -g -fno-omit-frame-pointer \ -O3 -g0

CFLAGS += "-I$(LVGL_DIR_NAME)/"
CFLAGS += "-I$(LVGL_DIR)/ui/generated"
CFLAGS += "-I$(LVGL_DIR)/ui/custom"
CFLAGS += "-I$(LVGL_DIR)/ui/generated/"
CFLAGS += "-I$(LVGL_DIR)/ui/dclock/"
CFLAGS += "-I$(LVGL_DIR)/ui/generated/i18n"
CFLAGS += "-I$(LVGL_DIR)/ui/generated/storageData/"
CFLAGS += "-I$(LVGL_DIR)/ui/generated/myTimer/"
CFLAGS += "-I$(LVGL_DIR)/ui/generated/uart/"
CFLAGS += "-I$(LVGL_DIR)/ui/generated/mppFileManager/"
CFLAGS += "-I$(LVGL_DIR)/ui/generated/bootLogo/"

CFLAGS += "-I$(LVGL_DIR)/ui/generated/MediaLibrary/"
CFLAGS += "-I$(LVGL_DIR)/ui/generated/OTA/"
CFLAGS += "-I$(LVGL_DIR)/ui/generated/tireAlarm/"

CFLAGS += "-I$(LVGL_DIR)/../external/aw_pack_src/lib_aw/include/eyesee-mpp/middleware/media/LIBRARY/libcedarc/include"

# CarPlay/Android Auto: 1=enable, 0=disable (should match runcarplay/prj/linux/makeconfig CONFIG_CARPLAY)
CONFIG_CARPLAY ?= 1

CFLAGS += "-I$(LVGL_DIR)/../runcarplay/src/carplay"
CFLAGS += "-I$(LVGL_DIR)/../runcarplay/src/carplay_display"
CFLAGS += "-I$(LVGL_DIR)/../runcarplay/src/bluetooth"
CFLAGS += "-I$(LVGL_DIR)/../runcarplay/src/tire"
CFLAGS += "-I$(LVGL_DIR)/../external/include"
CFLAGS += "-I$(LVGL_DIR)/../external/include/libcedarc"
CFLAGS += "-I$(LVGL_DIR)/../external/aw_pack_src/lib_aw/include/eyesee-mpp/system/public/include/vo"
CFLAGS += "-I$(LVGL_DIR)/lv_drivers/indev/"

CFLAGS += "-I$(LVGL_DIR)/../runcarplay/src/arch/linux"
CFLAGS += "-I$(LVGL_DIR)/../runcarplay/src/common"
CFLAGS += "-I$(LVGL_DIR)/../runcarplay/src/drivers/linux"
CFLAGS += "-I$(LVGL_DIR)/../runcarplay/src/app/video"
CFLAGS += "-I$(LVGL_DIR)/../external/include/libawlist"
CFLAGS += "-I$(LVGL_DIR)/../external/include/libawnn_full"
CFLAGS += "-I$(LVGL_DIR)/../external/include/libawaiisp"


CFLAGS += "-I$(LVGL_DIR)/../external/aw_pack_src/lib_aw/include/glogCWrapper"
CFLAGS += "-I$(LVGL_DIR)/../external/aw_pack_src/lib_aw/include/eyesee-mpp/middleware/include/media"
CFLAGS += "-I$(LVGL_DIR)/../external/aw_pack_src/lib_aw/include/eyesee-mpp/middleware/include/utils"
CFLAGS += "-I$(LVGL_DIR)/../external/aw_pack_src/lib_aw/include/eyesee-mpp/middleware/include"
CFLAGS += "-I$(LVGL_DIR)/../external/aw_pack_src/lib_aw/include/eyesee-mpp/middleware/media/include/utils"
CFLAGS += "-I$(LVGL_DIR)/../external/aw_pack_src/lib_aw/include"
# CFLAGS += "-I$(LVGL_DIR)/../external/aw_pack_src/lib_aw/include/glogCWrapper"
# CFLAGS += "-I$(LVGL_DIR)/../external/aw_pack_src/lib_aw/include/libawlist"
# CFLAGS += "-I$(LVGL_DIR)/../external/aw_pack_src/lib_aw/include/libawnn_full"
CFLAGS += "-I$(LVGL_DIR)/../external/aw_pack_src/lib_aw/include/eyesee-mpp/middleware/media/LIBRARY/include_stream"
CFLAGS += "-I$(LVGL_DIR)/../external/aw_pack_src/lib_aw/include/eyesee-mpp/middleware/media/include/component"
CFLAGS += "-I$(LVGL_DIR)/../external/aw_pack_src/lib_aw/include/eyesee-mpp/middleware/media/LIBRARY/include_FsWriter"
CFLAGS += "-I$(LVGL_DIR)/../external/aw_pack_src/lib_aw/include/eyesee-mpp/middleware/media/LIBRARY/libisp/include"
CFLAGS += "-I$(LVGL_DIR)/../external/aw_pack_src/lib_aw/include/eyesee-mpp/middleware/media/LIBRARY/libisp/isp_tuning"
CFLAGS += "-I$(LVGL_DIR)/../external/aw_pack_src/lib_aw/include/eyesee-mpp/middleware/media/LIBRARY/libisp/include/V4l2Camera"
CFLAGS += "-I$(LVGL_DIR)/../external/aw_pack_src/lib_aw/include/eyesee-mpp/middleware/media/LIBRARY/libisp"
CFLAGS += "-I$(LVGL_DIR)/../external/aw_pack_src/lib_aw/include/eyesee-mpp/middleware/media/LIBRARY/libisp/include/device"
CFLAGS += "-I$(LVGL_DIR)/../external/aw_pack_src/lib_aw/include/eyesee-mpp/middleware/media/LIBRARY/include_muxer"
# CFLAGS += "-I$(LVGL_DIR)/../external/aw_pack_src/lib_aw/include/eyesee-mpp/middleware/media/LIBRARY/libcedarc/include"

# CFLAGS += "-I$(LVGL_DIR)/../external/aw_pack_src/lib_aw/include/eyesee-mpp/middleware/media/LIBRARY/include_muxer"
# CFLAGS += "-I$(LVGL_DIR)/../external/aw_pack_src/lib_aw/include/eyesee-mpp/middleware/media/LIBRARY/libResample/include"
# CFLAGS += "-I$(LVGL_DIR)/../external/aw_pack_src/lib_aw/include/eyesee-mpp/middleware/sample/configfileparser"
# CFLAGS += "-I$(LVGL_DIR)/../external/aw_pack_src/lib_aw/include/eyesee-mpp/system/public/include"
# CFLAGS += "-I$(LVGL_DIR)/../external/aw_pack_src/lib_aw/include/eyesee-mpp/system/public/libion/include"
# CFLAGS += "-I$(LVGL_DIR)/../external/aw_pack_src/lib_aw/include/eyesee-mpp/system/private/rtsp/IPCProgram/interface"
CFLAGS += "-I$(LVGL_DIR)/../external/aw_pack_src/lib_aw/include/libbaselist"

ifneq ($(CONFIG_CARPLAY),0)
CFLAGS += -DENABLE_CARPLAY=1
endif

# SDCardLibrary
CFLAGS += "-I$(LVGL_DIR)/../runcarplay/src/app/SDCardLibrary/"
CFLAGS += "-I$(LVGL_DIR)/ui/generated/SDCardLibrary/"
CFLAGS += "-I$(LVGL_DIR)/ui/generated/SDCardLibrary/logic/"
CFLAGS += "-I$(LVGL_DIR)/ui/generated/SDCardLibrary/res/"


# 媒体库解耦：runcarplay media_work / TFCard / photo_view（与 runcarplay 编译时 -I 一致）
CFLAGS += "-I$(LVGL_DIR)/../runcarplay/src/media/media_work"
CFLAGS += "-I$(LVGL_DIR)/../runcarplay/src/media/media_tools/TFCard"
CFLAGS += "-I$(LVGL_DIR)/../runcarplay/src/media/media_tools/photo_view"


LDFLAGS = -lm -Wl,-gc-sections

TARGET = liblvgl.a
#BIN = lvgl_test


#Collect the files to compile
MAINSRC = ./lvgl_main.c

#-------------------------------------------
include $(LVGL_DIR)/ui/generated/generated.mk
include $(LVGL_DIR)/ui/generated/images/images.mk
include $(LVGL_DIR)/ui/generated/guider_fonts/guider_fonts.mk
include $(LVGL_DIR)/ui/custom/custom.mk
include $(LVGL_DIR)/ui/generated/guider_customer_fonts/guider_customer_fonts.mk

UI_CSRCS += $(wildcard ui/generated/*.c)
UI_CSRCS += $(wildcard ui/generated/guider_fonts/*.c)
UI_CSRCS += $(wildcard ui/custom/*.c)
UI_CSRCS += $(wildcard ui/generated/images/*.c)
UI_CSRCS += $(wildcard ui/dclock/*.c)
UI_CSRCS += $(wildcard ui/generated/i18n/*c)
UI_CSRCS += $(wildcard ui/generated/storageData/*c)
UI_CSRCS += $(wildcard ui/generated/myTimer/*c)
UI_CSRCS += $(wildcard ui/generated/uart/*c)
UI_CSRCS += $(wildcard ui/generated/mppFileManager/*c)
UI_CSRCS += $(wildcard ui/generated/bootLogo/*c)

UI_CSRCS += $(wildcard ui/generated/MediaLibrary/*c)
UI_CSRCS += $(wildcard ui/generated/MediaLibrary/res/*c)
UI_CSRCS += $(wildcard ui/generated/MediaLibrary/font/*c)
UI_CSRCS += $(wildcard ui/generated/OTA/*c)
UI_CSRCS += $(wildcard ui/generated/OTA/res/*c)
UI_CSRCS += $(wildcard ui/generated/OTA/font/*c)
UI_CSRCS += $(wildcard ui/generated/tireAlarm/*c)


CSRCS += $(UI_CSRCS)
UI_OBJS := $(UI_CSRCS:.C=.O)
OBJS += $(UI_OBJS)
#-------------------------------------------

include $(LVGL_DIR)/lvgl/lvgl.mk
include $(LVGL_DIR)/lv_drivers/lv_drivers.mk

OBJEXT ?= .o

AOBJS = $(ASRCS:.S=$(OBJEXT))
COBJS = $(CSRCS:.c=$(OBJEXT))

MAINOBJ = $(MAINSRC:.c=$(OBJEXT))

SRCS = $(ASRCS) $(CSRCS) $(MAINSRC)
OBJS = $(AOBJS) $(COBJS)


all: default

%.o: %.c 
	@$(CC)  $(CFLAGS) -c $< -o $@
	@echo "CC $<"
    

default:$(AOBJS) $(COBJS) $(MAINOBJ)
	$(AR) -crv $(TARGET) $(MAINOBJ) $(AOBJS) $(COBJS) 
	cp -rf ./$(TARGET) ../external/lib/

#default: $(AOBJS) $(COBJS) $(MAINOBJ)
#	$(CC) -o $(BIN) $(MAINOBJ) $(AOBJS) $(COBJS) $(LDFLAGS)

clean: 
	rm -rf $(BIN) $(AOBJS) $(COBJS) $(MAINOBJ)
	rm liblvgl.a
	rm ../external/lib/liblvgl.a
	
