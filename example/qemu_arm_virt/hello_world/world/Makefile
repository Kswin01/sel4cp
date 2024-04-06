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

WORLD_OBJS := world.o

BOARD_DIR := $(SEL4CP_SDK)/board/$(SEL4CP_BOARD)/$(SEL4CP_CONFIG)

IMAGES := world.elf
CFLAGS := -mcpu=$(CPU) -mstrict-align -nostdlib -ffreestanding -g3 -Wall  -Wno-unused-function -Werror -I$(BOARD_DIR)/include -fno-emulated-tls --target=aarch64-none-elf 
LDFLAGS := -L$(BOARD_DIR)/lib -Wl,-plugin-opt=-emulated-tls=0
LIBS := -l:libdummycrt.a -l:libsel4cp.a -Tdummycrt.ld

WORLD_BUILD_DIR = $(BUILD_DIR)/world

all: $(addprefix $(WORLD_BUILD_DIR)/, $(IMAGES))

$(WORLD_BUILD_DIR)/%.o: %.c Makefile
	$(CC) -c $(CFLAGS) $< -o $@

$(WORLD_BUILD_DIR)/world.elf: $(addprefix $(WORLD_BUILD_DIR)/, $(WORLD_OBJS))
	$(LD) $(LDFLAGS) $^ $(LIBS) -o $@
