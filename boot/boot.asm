org 0x7c00
bits 16

%define ENDL 0x0D, 0x0A

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

puts:
    push si
    push ax
.putsloop:
    lodsb
    or al, al
    jz .done

    mov ah, 0x0e
    mov bh, 0      
    int 0x10        ; bios interrupt (video)
    jmp .putsloop
.done:
    pop ax
    pop si
    ret


msg: db "Hello from zuzu386!", ENDL, 0

times 510 - ($ - $$) db 0 ; padding
dw 0xAA55