%define ENDL 0x0D, 0x0A

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
