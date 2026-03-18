_GDT:
.null:
    dq 0                        ; null descriptor, required

.code32:
    dw 0xFFFF                   ; limit (0-15)
    dw 0                        ; base (0-15)
    db 0                        ; base (16-23)
    db 10011010b                ; access
    db 11001111b                ; flags + limit (16-19)
    db 0                        ; base (24-31)

.data32:
    dw 0xFFFF                   ; limit (0-15)
    dw 0                        ; base (0-15)
    db 0                        ; base (16-23)
    db 10010010b                ; access
    db 11001111b                ; flags + limit (16-19)
    db 0                        ; base (24-31)

_GDT_end:

_GDT_descriptor:
    dw _GDT_end - _GDT - 1     ; size - 1
    dd _GDT                     ; address

setup_gdt:
    lgdt [_GDT_descriptor]
    ret