#
# Copyright 2021, Breakaway Consulting Pty. Ltd.
#
# SPDX-License-Identifier: BSD-2-Clause
#
ifeq ($(strip $(BUILD_DIR)),)
$(error BUILD_DIR must be specified)
endif

ifeq ($(strip $(GCC_CPU)),)
$(error GCC_CPU must be specified)
endif

TOOLCHAIN := clang 
CFLAGS := -std=gnu11 -g3 -O3 -nostdlib -ffreestanding -mcpu=$(GCC_CPU) -Wall -Wno-maybe-uninitialized -Wno-unused-function -Werror -Iinclude -I$(SEL4_SDK)/include -fno-emulated-tls --target=aarch64-none-elf

LIBS := libsel4cp
OBJS := main.o dbg.o
OTHER_OBJS :=

$(BUILD_DIR)/%.o : src/%.S
	$(TOOLCHAIN) -x assembler-with-cpp -c -g  -mcpu=$(GCC_CPU)  $< -o $@

$(BUILD_DIR)/%.o : src/%.s
	$(TOOLCHAIN) -g -mcpu=$(GCC_CPU) $< -o $@

$(BUILD_DIR)/%.o : src/%.c
	$(TOOLCHAIN) -c $(CFLAGS) $< -o $@

LIB = $(addprefix $(BUILD_DIR)/, $(LIBS).a)

all: $(LIB) $(addprefix $(BUILD_DIR)/, $(OTHER_OBJS))

$(LIB): $(addprefix $(BUILD_DIR)/, $(OBJS))
	llvm-ar -rv $@ $^
