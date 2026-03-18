# zuzu386

This kernel serves as a learning project for x86 architecture, focusing on the basics of bootloading, protected mode, and low-level hardware interaction, as a side project off the main [zuzu](https://github.com/) kernel, built specifically to explore x86 architecture.

---

## Architecture

- **Target:** i386 / 32-bit protected mode
- **Bootloader:** Custom
- **Kernel:** C/Assembly
- **Boot flow:** BIOS -> 16-bit real mode -> protected mode -> "kernel"

---

## Toolchain

| Tool      | Command                     |
| --------- | --------------------------- |
| Assembler | `nasm`                      |
| Compiler  | `x86_64-elf-gcc -m32`       |
| Linker    | `x86_64-elf-ld -m elf_i386` |
| Emulator  | `qemu-system-i386`          |

---

## Project Structure

```
zuzu386/
├── arch/
│   └── x86/
│       ├── boot/    # x86 bootloader (NASM)
│       └── linker.ld
├── kernel/      # kernel C code
├── include/     # headers
├── scripts/     # helper tooling
├── build/       # compiled output (gitignored)
└── Makefile
```

---

## Building

```sh
make
```

## Running in QEMU

```sh
make run
# or manually:
qemu-system-i386 -drive format=raw,file=build/os-image.bin
```

## Debugging (QEMU + GDB)

In terminal 1:

```sh
make debug
```

In terminal 2:

```sh
make debug-gdb
```

Defaults:
- GDB server port: `1234`
- Real-mode architecture preset: `i8086`

Override port if needed:

```sh
make debug GDB_PORT=9000
make debug-gdb GDB_PORT=9000
```

---

## Status

# zuzu386 Roadmap

A learning kernel. Scope is intentionally tiny — no userspace, no threads, no scheduler.

---

## Bootloader
- [x] 512-byte MBR bootloader in NASM
- [x] BIOS screen output via INT 10h
- [x] Proper boot signature (0xAA55)
- [x] Stack setup
- [x] Boot in QEMU successfully

## Protected mode entry
- [x] Define and load a basic GDT (code + data segments)
- [x] Flip the PE bit in cr0
- [x] Far jump to flush pipeline
- [x] Reload segment registers
- [x] Confirm 32-bit mode in QEMU

## Kernel entry
- [x] Basic linker script (ELF32)
- [x] Makefile wiring bootloader → kernel
- [x] `kernel_main()` in C
- [x] Bootloader hands off execution to kernel

## VGA
- [x] Write directly to 0xB8000
- [x] `print_string()` / `print_char()`
- [x] Clear screen
- [x] Scrolling
- [x] Basic color support

## IRQ
- [x] IDT setup
- [x] Basic exception handlers (divide by zero, GPF, etc.)
- [x] PIC (8259) remapping
- [x] Timer interrupt (IRQ0)

## Keyboard
- [x] Keyboard interrupt handler (IRQ1)
- [x] Scancode → ASCII mapping
- [x] Echo typed characters to screen

## If I'm really ambitious... (which I am probably)
- [ ] Tiny input loop (read a command, respond)
- [ ] Boot from USB on real hardware
- [ ] Memory map via BIOS (INT 0x15 E820) before mode switch

---

## Out of scope
- Userspace
- Threads / scheduling
- Filesystem
- Memory allocator


---

## Resources

- [OSDev Wiki](https://wiki.osdev.org)
- [Nanobyte OS Dev Series](https://www.youtube.com/@nanobyte-dev)
- *Writing a Simple OS from Scratch* — Nick Blundell