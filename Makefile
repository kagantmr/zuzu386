# Generic Makefile for a small x86 OS project

ARCH ?= i386
CROSS ?= i686-elf-

# If i686-elf tools are missing, try common alternatives.
ifneq ($(shell command -v $(CROSS)gcc >/dev/null 2>&1; echo $$?),0)
ifeq ($(shell command -v i686-linux-gnu-gcc >/dev/null 2>&1; echo $$?),0)
CROSS := i686-linux-gnu-
else
ifeq ($(shell command -v x86_64-elf-gcc >/dev/null 2>&1; echo $$?),0)
CROSS := x86_64-elf-
endif
endif
endif

CC ?= $(CROSS)gcc
LD ?= $(CROSS)ld
OBJCOPY ?= $(CROSS)objcopy
NASM ?= nasm

# If CC/LD/OBJCOPY are set externally to missing tools, recover with the resolved CROSS prefix.
ifneq ($(shell command -v $(CC) >/dev/null 2>&1; echo $$?),0)
ifeq ($(shell command -v $(CROSS)gcc >/dev/null 2>&1; echo $$?),0)
CC := $(CROSS)gcc
endif
endif
ifneq ($(shell command -v $(LD) >/dev/null 2>&1; echo $$?),0)
ifeq ($(shell command -v $(CROSS)ld >/dev/null 2>&1; echo $$?),0)
LD := $(CROSS)ld
endif
endif
ifneq ($(shell command -v $(OBJCOPY) >/dev/null 2>&1; echo $$?),0)
ifeq ($(shell command -v $(CROSS)objcopy >/dev/null 2>&1; echo $$?),0)
OBJCOPY := $(CROSS)objcopy
endif
endif

# If environment defaults force host compiler/linker names, switch to cross tools when available.
ifeq ($(notdir $(CC)),cc)
ifeq ($(shell command -v $(CROSS)gcc >/dev/null 2>&1; echo $$?),0)
CC := $(CROSS)gcc
endif
endif
ifeq ($(notdir $(LD)),ld)
ifeq ($(shell command -v $(CROSS)ld >/dev/null 2>&1; echo $$?),0)
LD := $(CROSS)ld
endif
endif

# Fallbacks for hosts without a full cross toolchain (useful on macOS).
ifneq ($(shell command -v $(LD) >/dev/null 2>&1; echo $$?),0)
LD := ld.lld
endif
ifneq ($(shell command -v $(OBJCOPY) >/dev/null 2>&1; echo $$?),0)
OBJCOPY := llvm-objcopy
endif
QEMU ?= qemu-system-$(ARCH)
QEMU_PCSPK_FLAGS ?= -audiodev coreaudio,id=snd0 -machine pcspk-audiodev=snd0
QEMU_LOG ?= $(BUILD_DIR)/qemu.log
QEMU_TRACE_FLAGS ?= int,cpu_reset,guest_errors
BOCHS ?= bochs
BOCHSDBG ?= bochsdbg
BOCHS_LOG ?= $(BUILD_DIR)/bochs.log
BOCHS_DEBUG_LOG ?= $(BUILD_DIR)/bochsdbg.log
BOCHSRC ?= $(BUILD_DIR)/bochsrc.txt
BOCHS_SHARE ?= $(firstword $(wildcard /opt/homebrew/share/bochs /usr/local/share/bochs /usr/share/bochs))
BOCHS_BIOS ?= $(BOCHS_SHARE)/BIOS-bochs-latest
BOCHS_VGABIOS ?= $(BOCHS_SHARE)/VGABIOS-lgpl-latest
GDB ?= gdb
GDB_PORT ?= 1234
DEBUG_QEMU_FLAGS ?= -S -no-reboot -no-shutdown

BUILD_DIR := build
ARCH_DIR := arch/x86
ARCH_SRC_DIR := $(ARCH_DIR)
BOOT_DIR := $(ARCH_DIR)/boot
KERNEL_DIR := kernel
INCLUDE_DIR := include

# Expected files
BOOT_SECTOR ?= $(BOOT_DIR)/boot.asm
LINKER_SCRIPT ?= $(ARCH_DIR)/linker.ld

# Source discovery
KERNEL_C_SRCS := $(shell find $(KERNEL_DIR) -type f -name '*.c')
KERNEL_S_SRCS := $(shell find $(KERNEL_DIR) -type f -name '*.S')
ARCH_C_SRCS := $(shell find $(ARCH_SRC_DIR) -type f -name '*.c')
ARCH_S_SRCS := $(shell find $(ARCH_SRC_DIR) -type f -name '*.S')
# Standalone NASM module(s) linked into the kernel ELF.
BOOT_ELF_ASM_SRCS ?= $(BOOT_DIR)/kernel_entry.asm

KERNEL_C_OBJS := $(patsubst $(KERNEL_DIR)/%.c,$(BUILD_DIR)/kernel/%.o,$(KERNEL_C_SRCS))
KERNEL_S_OBJS := $(patsubst $(KERNEL_DIR)/%.S,$(BUILD_DIR)/kernel/%.o,$(KERNEL_S_SRCS))
ARCH_C_OBJS := $(patsubst $(ARCH_SRC_DIR)/%.c,$(BUILD_DIR)/arch/%.o,$(ARCH_C_SRCS))
ARCH_S_OBJS := $(patsubst $(ARCH_SRC_DIR)/%.S,$(BUILD_DIR)/arch/%.o,$(ARCH_S_SRCS))
BOOT_ELF_ASM_OBJS := $(patsubst $(BOOT_DIR)/%.asm,$(BUILD_DIR)/boot/%.o,$(BOOT_ELF_ASM_SRCS))

# Entry object MUST come first so _start lands at the load address.
KERNEL_OBJS := $(BOOT_ELF_ASM_OBJS) $(ARCH_C_OBJS) $(ARCH_S_OBJS) $(KERNEL_C_OBJS) $(KERNEL_S_OBJS)

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
KERNEL_LOAD_INC := $(BUILD_DIR)/kernel_load.inc
RAMDISK := $(BUILD_DIR)/ramdisk.zrd
MIDI_DIR := midi
MIDI_FILES := $(wildcard $(MIDI_DIR)/*.mid)
ZUZM_FILES := $(patsubst $(MIDI_DIR)/%.mid,$(BUILD_DIR)/%.zuzm,$(MIDI_FILES))
USB_BS ?= 1M
DISK ?=

IMAGE_PARTS := $(BOOT_BIN)
ifeq ($(HAS_KERNEL),1)
IMAGE_PARTS += $(KERNEL_BIN)
endif
IMAGE_PARTS += $(RAMDISK)

# Build flags
CFLAGS ?= -m32 -mgeneral-regs-only -ffreestanding -fno-pie -fno-stack-protector -nostdlib -nostdinc -Wall -Wextra -O2 -mno-mmx -mno-sse -mno-sse2 -I. -I$(INCLUDE_DIR) -I$(KERNEL_DIR)
ASFLAGS_ELF ?= -f elf32 -I$(BOOT_DIR)/ -I$(INCLUDE_DIR)/ -I$(BUILD_DIR)/
ASFLAGS_BIN ?= -f bin -I$(BOOT_DIR)/ -I$(INCLUDE_DIR)/ -I$(BUILD_DIR)/
LDFLAGS_BASE := -m elf_i386

# On macOS, cc is usually clang; force an i386-ELF target for freestanding output.
ifneq ($(strip $(shell $(CC) --version 2>/dev/null | head -n 1 | grep -Ei "clang|Apple clang")),)
CFLAGS += --target=i386-elf
endif

ifeq ($(wildcard $(LINKER_SCRIPT)),)
LDFLAGS := $(LDFLAGS_BASE)
else
LDFLAGS := $(LDFLAGS_BASE) -T $(LINKER_SCRIPT)
endif

.PHONY: all run debug debug-gdb qemu-log qemu-log-check bochs bochsdbg clean help usb-list usb-write

all: $(OS_IMAGE)

$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/kernel/%.o: $(KERNEL_DIR)/%.c | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/kernel/%.o: $(KERNEL_DIR)/%.S | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/arch/%.o: $(ARCH_SRC_DIR)/%.c | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/arch/%.o: $(ARCH_SRC_DIR)/%.S | $(BUILD_DIR)
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

ifeq ($(HAS_KERNEL),1)
$(KERNEL_LOAD_INC): $(KERNEL_BIN) $(RAMDISK) | $(BUILD_DIR)
	@ksize=$$(wc -c < "$(KERNEL_BIN)"); \
	ksectors=$$(((ksize + 511) / 512)); \
	if [ $$ksectors -eq 0 ]; then ksectors=1; fi; \
	rsize=$$(wc -c < "$(RAMDISK)"); \
	rsectors=$$(((rsize + 511) / 512)); \
	if [ $$rsectors -eq 0 ]; then rsectors=1; fi; \
	(printf '%%define KERNEL_LOAD_SECTORS %s\n' "$$ksectors"; \
	 printf '%%define KERNEL_SIZE_BYTES %s\n' "$$ksize"; \
	 printf '%%define RAMDISK_LOAD_SECTORS %s\n' "$$rsectors"; \
	 printf '%%define RAMDISK_SIZE_BYTES %s\n' "$$rsize") > $@
else
$(KERNEL_LOAD_INC): | $(BUILD_DIR)
	@printf '%%define KERNEL_LOAD_SECTORS 0\n' > $@
endif

$(BOOT_BIN): $(BOOT_SECTOR) $(KERNEL_LOAD_INC) | $(BUILD_DIR)
	$(NASM) $(ASFLAGS_BIN) $< -o $@

# Convert MIDI files to binary format
$(BUILD_DIR)/%.zuzm: $(MIDI_DIR)/%.mid | $(BUILD_DIR)
	@python3 -c "import mido" 2>/dev/null && \
		python3 scripts/midi_to_zuzu.py $< $@ || \
		python3 scripts/zuzm_gen.py $@ 120

$(RAMDISK): $(ZUZM_FILES) | $(BUILD_DIR)
	python3 scripts/build_zrd.py $@ $(ZUZM_FILES)

$(OS_IMAGE): $(IMAGE_PARTS) | $(BUILD_DIR)
	cat $^ > $@
	@if [ "$(HAS_KERNEL)" = "0" ]; then \
		echo "Built boot-only image (no kernel sources detected)."; \
	fi
	@echo "Built $(OS_IMAGE)"

$(BOCHSRC): $(OS_IMAGE) | $(BUILD_DIR)
	@if [ ! -f "$(BOCHS_BIOS)" ] || [ ! -f "$(BOCHS_VGABIOS)" ]; then \
		echo "Bochs BIOS files not found."; \
		echo "Set BOCHS_BIOS=... and BOCHS_VGABIOS=... or install bochs via Homebrew."; \
		exit 1; \
	fi
	@printf '%s\n' \
		'megs: 32' \
		'romimage: file=$(BOCHS_BIOS)' \
		'vgaromimage: file=$(BOCHS_VGABIOS)' \
		'boot: disk' \
		'log: $(BOCHS_LOG)' \
		'panic: action=ask' \
		'error: action=report' \
		'info: action=report' \
		'ata0-master: type=disk, mode=flat, path="$(abspath $(OS_IMAGE))", cylinders=1, heads=16, spt=63' \
		'clock: sync=realtime, time0=local' \
		'mouse: enabled=0' \
		'keyboard: type=mf' \
		'port_e9_hack: enabled=1' \
		> $@

run: $(OS_IMAGE)
	$(QEMU) $(QEMU_PCSPK_FLAGS) -drive format=raw,file=$(OS_IMAGE)

debug: $(OS_IMAGE)
	$(QEMU) -drive format=raw,file=$(OS_IMAGE) -gdb tcp::$(GDB_PORT) $(DEBUG_QEMU_FLAGS)

debug-gdb:
	$(GDB) -ex "set architecture i8086" -ex "target remote :$(GDB_PORT)"

qemu-log: $(OS_IMAGE)
	@rm -f $(QEMU_LOG)
	$(QEMU) -drive format=raw,file=$(OS_IMAGE) -no-reboot -no-shutdown -d $(QEMU_TRACE_FLAGS) -D $(QEMU_LOG)

bochs: $(BOCHSRC)
	@command -v $(BOCHS) >/dev/null 2>&1 || { echo "Bochs not found. Install it first, for example: brew install bochs"; exit 1; }
	$(BOCHS) -q -f $(BOCHSRC)

bochsdbg: $(BOCHSRC)
	@command -v $(BOCHSDBG) >/dev/null 2>&1 || { echo "bochsdbg not found. Install it first, for example: brew install bochs"; exit 1; }
	$(BOCHSDBG) -q -f $(BOCHSRC)

qemu-log-check:
	@if [ ! -f "$(QEMU_LOG)" ]; then \
		echo "No log file found at $(QEMU_LOG). Run 'make qemu-log' first."; \
		exit 1; \
	fi
	@if grep -Eqi 'triple[ -]?fault' $(QEMU_LOG); then \
		echo "Triple-fault signature found in $(QEMU_LOG)."; \
		grep -Ein 'triple[ -]?fault' $(QEMU_LOG); \
	else \
		echo "No explicit triple-fault signature found in $(QEMU_LOG)."; \
	fi

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
	@echo "  debug  - Run in QEMU paused with gdb server on GDB_PORT"
	@echo "  debug-gdb - Attach GDB to the QEMU gdb server"
	@echo "  qemu-log - Run QEMU and write trace log to QEMU_LOG"
	@echo "  qemu-log-check - Scan QEMU_LOG for triple-fault signatures"
	@echo "  bochs  - Run the image in Bochs"
	@echo "  bochsdbg - Run the image in the Bochs debugger"
	@echo "  usb-list  - List removable disks"
	@echo "  usb-write - Write image to USB (requires DISK=... CONFIRM=YES)"
	@echo "  clean  - Remove build artifacts"
	@echo ""
	@echo "Configurable vars:"
	@echo "  CROSS=$(CROSS)"
	@echo "  ARCH=$(ARCH)"
	@echo "  NASM=$(NASM)"
	@echo "  QEMU_LOG=$(QEMU_LOG)"
	@echo "  QEMU_TRACE_FLAGS=$(QEMU_TRACE_FLAGS)"
	@echo "  BOCHS=$(BOCHS)"
	@echo "  BOCHSDBG=$(BOCHSDBG)"
	@echo "  BOCHS_BIOS=$(BOCHS_BIOS)"
	@echo "  BOCHS_VGABIOS=$(BOCHS_VGABIOS)"
	@echo "  BOCHSRC=$(BOCHSRC)"
	@echo "  GDB=$(GDB)"
	@echo "  GDB_PORT=$(GDB_PORT)"
	@echo "  DEBUG_QEMU_FLAGS=$(DEBUG_QEMU_FLAGS)"
	@echo "  QEMU_PCSPK_FLAGS=$(QEMU_PCSPK_FLAGS)"
	@echo "  BOOT_SECTOR=$(BOOT_SECTOR)"
	@echo "  LINKER_SCRIPT=$(LINKER_SCRIPT)"
	@echo "  HAS_KERNEL=$(HAS_KERNEL)"
	@echo "  DISK=$(DISK)"
	@echo "  USB_BS=$(USB_BS)"
