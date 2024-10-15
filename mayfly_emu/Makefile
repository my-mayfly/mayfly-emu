BUILD_DIR = ./build
SO = 
SHARE = 0
WORK_DIR  = $(shell pwd)
NAME = mayfly
BINARY = $(BUILD_DIR)/$(NAME)$(SO)
INC_PATH  := $(WORK_DIR)/include $(INC_PATH)
OBJ_DIR   := $(BUILD_DIR)/obj-$(NAME)$(SO)

SRC_DIRS := ./cpu ./device ./memory ./monitor ./system ./
ifeq ($(SHARE),1)
SO := -so 
CFLAGS += -fPIC
LDFLAGS += -rdynamic -shared -fPIC 
endif 

CC = gcc
# Compilation flags
ifeq ($(CC),clang)
CXX := clang++
else 
CXX := g++
endif 
LD := $(CXX)
INCLUDES = $(addprefix -I, $(INC_PATH))
CFLAGS :=  -O2 -MMD -Wall -Werror $(INCLUDES) $(CFLAGS) 
LDFLAGS :=  -O2 $(LDFLAGS)
LDFLAGS += -lpthread -lSDL2 -ldl -lpcre -lreadline -lrt
CXXFLAGS += $(CFLAGS) -ffreestanding -fno-rtti -fno-exceptions $(shell llvm-config --cxxflags) -fPIE 
CSRCS := $(wildcard $(addsuffix /*.c, $(SRC_DIRS)))
OBJS := $(patsubst %.c, $(OBJ_DIR)/%.o, $(notdir $(CSRCS))) 
ifneq ($(SHARE),1)
OBJS += $(OBJ_DIR)/disasm.o
endif
VPATH += $(SRC_DIRS) ./utils

LDFLAGS += $(shell llvm-config --libs)

BIN_PATH_DIR = $(shell pwd)
#IMG_FILE ?= --img=/home/xingk/risc-v_w_H/linux_bin/fw_payload.bin
IMG_FILE ?= --img=$(BIN_PATH_DIR)/fw_payload.bin
#KERNEL_FILE ?= --kernel=/home/xingk/risc-v_w_H/linux_bin/Image
KERNEL_FILE ?= --kernel=$(BIN_PATH_DIR)/Image
SYSTEM_FILE ?= --file-system=$(BIN_PATH_DIR)/rootfs2.img
# Compilation patterns
$(OBJ_DIR)/%.o: %.c
	echo + CC $<
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: %.cc
	echo + CXX $<
	@mkdir -p $(dir $@)
	@$(CXX) $(CFLAGS) $(CXXFLAGS) -c -o $@ $<


$(BINARY): $(OBJS)
	@echo + LD $@
	@$(LD) -o $@ $(OBJS) $(LDFLAGS)

run:$(BINARY)
	$(BINARY) $(IMG_FILE) $(KERNEL_FILE) $(SYSTEM_FILE) 

# 记得 -g ,与-O2 参数问题，自己搜索吧，写文档太难描述，ε=(´ο｀*)))唉。摆烂
gdb:$(BINARY)
	gdb --args $(BINARY) $(IMG_FILE) $(KERNEL_FILE) $(SYSTEM_FILE)

clean:
	rm -rf ./build
