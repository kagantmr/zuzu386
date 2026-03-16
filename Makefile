# Generic Makefile for a small x86 OS project

ARCH ?= i386
CROSS ?= i686-elf-

CC ?= $(CROSS)gcc
LD ?= $(CROSS)ld
OBJCOPY ?= $(CROSS)objcopy
NASM ?= nasm
QEMU ?= qemu-system-$(ARCH)
QEMU_PCSPK_FLAGS ?= -audiodev coreaudio,id=snd0 -machine pcspk-audiodev=snd0

BUILD_DIR := build
BOOT_DIR := boot
KERNEL_DIR := kernel
INCLUDE_DIR := include

# Expected files
BOOT_SECTOR ?= $(BOOT_DIR)/boot.asm
LINKER_SCRIPT ?= $(KERNEL_DIR)/linker.ld

# Source discovery
KERNEL_C_SRCS := $(wildcard $(KERNEL_DIR)/*.c)
KERNEL_S_SRCS := $(wildcard $(KERNEL_DIR)/*.S)
# Optional standalone boot asm modules to link into kernel. Keep empty by default
# so helper includes like boot/puts.asm are not treated as linkable objects.
BOOT_ELF_ASM_SRCS ?=

KERNEL_C_OBJS := $(patsubst $(KERNEL_DIR)/%.c,$(BUILD_DIR)/kernel/%.o,$(KERNEL_C_SRCS))
KERNEL_S_OBJS := $(patsubst $(KERNEL_DIR)/%.S,$(BUILD_DIR)/kernel/%.o,$(KERNEL_S_SRCS))
BOOT_ELF_ASM_OBJS := $(patsubst $(BOOT_DIR)/%.asm,$(BUILD_DIR)/boot/%.o,$(BOOT_ELF_ASM_SRCS))

KERNEL_OBJS := $(KERNEL_C_OBJS) $(KERNEL_S_OBJS) $(BOOT_ELF_ASM_OBJS)

ifneq ($(strip $(KERNEL_OBJS)),)
HAS_KERNEL := 1
else
HAS_KERNEL := 0
endif

# Output artifacts
KERNEL_ELF := $(BUILD_DIR)/kernel.elf
KERNEL_BIN := $(BUILD_DIR)/kernel.bin
BOOT_BIN := $(BUILD_DIR)/boot.bin
OS_IMAGE := $(BUILD_DIR)/os-image.bin
USB_BS ?= 1m
DISK ?=

IMAGE_PARTS := $(BOOT_BIN)
ifeq ($(HAS_KERNEL),1)
IMAGE_PARTS += $(KERNEL_BIN)
endif

# Build flags
CFLAGS ?= -m32 -ffreestanding -fno-pie -fno-stack-protector -nostdlib -nostdinc -Wall -Wextra -O2 -I$(INCLUDE_DIR)
ASFLAGS_ELF ?= -f elf32 -I$(BOOT_DIR)/ -I$(INCLUDE_DIR)/
ASFLAGS_BIN ?= -f bin -I$(BOOT_DIR)/ -I$(INCLUDE_DIR)/
LDFLAGS_BASE := -m elf_i386 -nostdlib

ifeq ($(wildcard $(LINKER_SCRIPT)),)
LDFLAGS := $(LDFLAGS_BASE)
else
LDFLAGS := $(LDFLAGS_BASE) -T $(LINKER_SCRIPT)
endif

.PHONY: all run run-speaker debug clean help usb-list usb-write

all: $(OS_IMAGE)

$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/kernel/%.o: $(KERNEL_DIR)/%.c | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/kernel/%.o: $(KERNEL_DIR)/%.S | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/boot/%.o: $(BOOT_DIR)/%.asm | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	$(NASM) $(ASFLAGS_ELF) $< -o $@

$(KERNEL_ELF): $(KERNEL_OBJS) | $(BUILD_DIR)
	@if [ -z "$(strip $(KERNEL_OBJS))" ]; then \
		echo "No kernel sources found in $(KERNEL_DIR)/ or ELF asm files in $(BOOT_DIR)/"; \
		echo "Add kernel sources or build image via the default boot-only path (make all/run)."; \
		exit 1; \
	fi
	$(LD) $(LDFLAGS) -o $@ $(KERNEL_OBJS)

$(KERNEL_BIN): $(KERNEL_ELF) | $(BUILD_DIR)
	$(OBJCOPY) -O binary $< $@

$(BOOT_BIN): $(BOOT_SECTOR) | $(BUILD_DIR)
	$(NASM) $(ASFLAGS_BIN) $< -o $@

$(OS_IMAGE): $(IMAGE_PARTS) | $(BUILD_DIR)
	cat $^ > $@
	@if [ "$(HAS_KERNEL)" = "0" ]; then \
		echo "Built boot-only image (no kernel sources detected)."; \
	fi
	@echo "Built $(OS_IMAGE)"

run: $(OS_IMAGE)
	$(QEMU) -drive format=raw,file=$(OS_IMAGE)

run-speaker: $(OS_IMAGE)
	$(QEMU) $(QEMU_PCSPK_FLAGS) -drive format=raw,file=$(OS_IMAGE)

debug: $(OS_IMAGE)
	$(QEMU) -drive format=raw,file=$(OS_IMAGE) -s -S

usb-list:
	@echo "Available removable disks:"
	@if [ "$$(uname)" = "Darwin" ]; then \
		diskutil list external physical; \
	else \
		lsblk -o NAME,SIZE,TYPE,MODEL,TRAN,HOTPLUG,RM | grep -E 'disk|loop' || true; \
	fi

usb-write: $(OS_IMAGE)
	@if [ -z "$(DISK)" ]; then \
		echo "Set target disk explicitly. Example:"; \
		echo "  make usb-write DISK=/dev/disk4 CONFIRM=YES"; \
		exit 1; \
	fi
	@if [ "$(CONFIRM)" != "YES" ]; then \
		echo "Refusing to write without explicit confirmation."; \
		echo "Re-run with CONFIRM=YES after verifying DISK=$(DISK)."; \
		exit 1; \
	fi
	@if [ "$$(uname)" = "Darwin" ]; then \
		RAW_DISK="$$(echo $(DISK) | sed 's|^/dev/disk|/dev/rdisk|')"; \
		echo "Unmounting $$RAW_DISK (and parent disk) ..."; \
		diskutil unmountDisk $(DISK); \
		echo "Writing $(OS_IMAGE) -> $$RAW_DISK"; \
		dd if=$(OS_IMAGE) of="$$RAW_DISK" bs=$(USB_BS) conv=sync; \
		sync; \
		echo "Done. Eject with: diskutil eject $(DISK)"; \
	else \
		echo "Writing $(OS_IMAGE) -> $(DISK)"; \
		sudo dd if=$(OS_IMAGE) of=$(DISK) bs=$(USB_BS) conv=fsync status=progress; \
		sync; \
		echo "Done."; \
	fi

clean:
	rm -rf $(BUILD_DIR)

help:
	@echo "Targets:"
	@echo "  all    - Build bootable image ($(OS_IMAGE))"
	@echo "  run    - Build and run in QEMU"
	@echo "  run-speaker - Run in QEMU with PC speaker audio enabled"
	@echo "  debug  - Run in QEMU with gdb stub (-s -S)"
	@echo "  usb-list  - List removable disks"
	@echo "  usb-write - Write image to USB (requires DISK=... CONFIRM=YES)"
	@echo "  clean  - Remove build artifacts"
	@echo ""
	@echo "Configurable vars:"
	@echo "  CROSS=$(CROSS)"
	@echo "  ARCH=$(ARCH)"
	@echo "  NASM=$(NASM)"
	@echo "  QEMU_PCSPK_FLAGS=$(QEMU_PCSPK_FLAGS)"
	@echo "  BOOT_SECTOR=$(BOOT_SECTOR)"
	@echo "  LINKER_SCRIPT=$(LINKER_SCRIPT)"
	@echo "  HAS_KERNEL=$(HAS_KERNEL)"
	@echo "  DISK=$(DISK)"
	@echo "  USB_BS=$(USB_BS)"
