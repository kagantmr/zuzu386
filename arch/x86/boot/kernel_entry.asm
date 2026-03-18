bits 32

global _start
extern kmain

section .text
_start:
    ; Set up a known-good stack for C code.
    mov esp, 0x90000
    call kmain

.hang:
    cli
    hlt
    jmp .hang
