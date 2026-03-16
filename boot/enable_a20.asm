; returns ZF set if A20 is disabled, ZF clear if enabled
test_a20:
    push ax
    push es
    push di
    push ds
    push si

    xor ax, ax
    mov ds, ax
    mov si, 0x0500          ; low address

    mov ax, 0xffff
    mov es, ax
    mov di, 0x0510          ; 0xffff:0x0510 = physical 0x100500

    mov al, [ds:si]         ; save original values
    push ax
    mov al, [es:di]
    push ax

    mov byte [ds:si], 0x00  ; write different values
    mov byte [es:di], 0xFF

    cmp byte [ds:si], 0xFF  ; if they match, A20 is off (wrapping)

    pop ax                  ; restore original values
    mov [es:di], al
    pop ax
    mov [ds:si], al

    pop si
    pop ds
    pop di
    pop es
    pop ax
    ret
    
enable_a20:
    push ax

    in al, 0x92
    or al, 0x02
    out 0x92, al

    call test_a20
    jnz .done               ; ZF clear = A20 is on, we're good

    ; fast method failed, try BIOS
    mov ax, 0x2403
    int 0x15
    jc .done
    test ah, ah
    jnz .done

    mov ax, 0x2402
    int 0x15
    jc .done
    test ah, ah
    jnz .done
    test al, al
    jnz .done

    mov ax, 0x2401
    int 0x15
    jc .done
    test ah, ah
    jnz .done

    ; Re-check after BIOS enable attempt.
    call test_a20
.done:
    pop ax
    ret
