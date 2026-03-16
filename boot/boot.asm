org 0x7c00
bits 16

jmp main ; should stay here no matter what

%define ENDL 0x0D, 0x0A
%include "puts.asm"

; bootloader
main:
    mov ax, 0
    mov ds, ax
    mov es, ax
    
    mov ss, ax
    mov sp, 0x7c00

    mov si, msg
    call puts

.halt:
    jmp .halt



msg: db "Starting zuzu386...", ENDL, 0

times 510 - ($ - $$) db 0 ; padding
dw 0xAA55