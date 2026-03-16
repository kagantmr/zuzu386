org 0x7c00
bits 16

jmp main ; should stay here no matter what

%include "enable_a20.asm"
%include "setup_gdt.asm"
%include "puts.asm"

; bootloader
main:
    mov ax, 0      ; clear segments
    mov ds, ax
    mov es, ax
    
    mov ss, ax     ; clear stack
    mov sp, 0x7c00 ; stack grows down so it wont overwrite the bootloader

    mov si, msg
    call puts
protected_mode_entry:
    cli

    push es
    mov ax, 0xB800
    mov es, ax

    mov byte [es:0x00], '1'
    mov byte [es:0x01], 0x0F

    call enable_a20

    mov byte [es:0x02], '2'
    mov byte [es:0x03], 0x0F

    call setup_gdt

    mov byte [es:0x04], '3'
    mov byte [es:0x05], 0x0F

    mov eax, cr0
    or eax, 0x1
    mov cr0, eax

    mov byte [es:0x06], '4'
    mov byte [es:0x07], 0x0F

    pop es
    jmp 0x08:.pmode
.pmode:
    bits 32
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov fs, ax
    mov gs, ax
    mov esp, 0x90000
.halt:
    jmp .halt



msg: db "zuzu386 is currently in 16-bit Real Mode", ENDL, 0

times 510 - ($ - $$) db 0 ; padding
dw 0xAA55