ifeq ($(strip $(BUILD_DIR)),)
$(error BUILD_DIR must be specified)
endif

ifeq ($(strip $(SEL4CP_SDK)),)
$(error SEL4CP_SDK must be specified)
endif

ifeq ($(strip $(SEL4CP_BOARD)),)
$(error SEL4CP_BOARD must be specified)
endif

ifeq ($(strip $(SEL4CP_CONFIG)),)
$(error SEL4CP_CONFIG must be specified)
endif

TOOLCHAIN := clang

CPU := cortex-a53

CC := $(TOOLCHAIN)
LD := $(TOOLCHAIN)
SEL4CP_TOOL ?= $(SEL4CP_SDK)/bin/sel4cp

HELLO_OBJS := hello.o

BOARD_DIR := $(SEL4CP_SDK)/board/$(SEL4CP_BOARD)/$(SEL4CP_CONFIG)

IMAGES := hello.elf
CFLAGS := -mcpu=$(CPU) -mstrict-align -nostdlib -ffreestanding -g3 -Wall  -Wno-unused-function -Werror -I$(BOARD_DIR)/include -fno-emulated-tls --target=aarch64-none-elf 
LDFLAGS := -L$(BOARD_DIR)/lib -Wl,-plugin-opt=-emulated-tls=0
LIBS := -l:libdummycrt.a -l:libsel4cp.a -Tdummycrt.ld

HELLO_BUILD_DIR = $(BUILD_DIR)/hello

all: $(addprefix $(HELLO_BUILD_DIR)/, $(IMAGES))

$(HELLO_BUILD_DIR)/%.o: %.c Makefile
	$(CC) -c $(CFLAGS) $< -o $@

$(HELLO_BUILD_DIR)/%.o: %.s Makefile
	$(AS) -g3 -mcpu=$(CPU) $< -o $@

$(HELLO_BUILD_DIR)/hello.elf: $(addprefix $(HELLO_BUILD_DIR)/, $(HELLO_OBJS))
	$(LD) $(LDFLAGS) $^ $(LIBS) -o $@
