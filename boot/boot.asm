org 0x7c00
bits 16

%define KERNEL_LOAD_SEGMENT 0x1000
%define KERNEL_ENTRY_POINT  0x10000
%include "kernel_load.inc"

; FAT12 HEADER
jmp main
nop
bdb_oem: db "MSWIN4.1" ; compatibility with some drivers that check for this string
bdb_bytes_per_sector: dw 512
bdb_sectors_per_cluster: db 1
bdb_reserved_sectors: dw 1
bdb_num_fats: db 2
bdb_root_entries: dw 224
bdb_total_sectors_short: dw 2880
bdb_media_descriptor: db 0xF0
bdb_sectors_per_fat: dw 9
bdb_sectors_per_track: dw 18
bdb_num_heads: dw 2
bdb_hidden_sectors: dd 0
bdb_total_sectors_long: dd 0

; EBR HEADER
ebr_drive_number: db 0
ebr_reserved: db 0
ebr_signature: db 0x29
ebr_volume_id: dd 0x12345678
ebr_volume_label: db "ZUZU 386   "
ebr_file_system_type: db "FAT12   ", 0


%include "enable_a20.asm"
%include "setup_gdt.asm"
; bootloader
main:
    mov [boot_drive], dl    ; save immediately
    mov ax, 0      ; clear segments
    mov ds, ax
    mov es, ax
    
    mov ss, ax     ; clear stack
    mov sp, 0x7c00 ; stack grows down so it wont overwrite the bootloader

load_kernel:
    ; check if LBA is supported
    mov ah, 0x41
    mov bx, 0x55AA
    int 0x13
    jc .no_lba
    cmp bx, 0xAA55
    jne .no_lba

    ; use LBA to read the kernel payload immediately after the boot sector
    mov ah, 0x42
    mov dl, [boot_drive]
    mov si, dap
    int 0x13
    jc disk_error
    jmp protected_mode_entry

.no_lba:
    ; CHS fallback: read one sector (works for tiny kernels on old BIOSes)
    mov ah, 0x02
    mov al, KERNEL_LOAD_SECTORS
    mov ch, 0x00
    mov cl, 0x02
    mov dh, 0x00
    mov dl, [boot_drive]
    mov ax, KERNEL_LOAD_SEGMENT
    mov es, ax
    mov bx, 0x0000
    int 0x13
    jc disk_error

protected_mode_entry:
    cli

    call enable_a20

    call setup_gdt

    mov eax, cr0
    or eax, 0x1
    mov cr0, eax
    jmp 0x08:protected_mode


protected_mode:
    bits 32
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov fs, ax
    mov gs, ax
    mov esp, 0x90000
    jmp KERNEL_ENTRY_POINT

disk_error:
    cli
disk_hang:
    hlt
    jmp disk_hang

boot_drive: db 0

dap:
    db 0x10                 ; size of DAP
    db 0                    ; reserved
    dw KERNEL_LOAD_SECTORS  ; sectors to read
    dw 0x0000               ; buffer offset
    dw KERNEL_LOAD_SEGMENT  ; buffer segment (loads to 0x10000)
    dq 1                    ; LBA start sector

times 510 - ($ - $$) db 0 ; padding
dw 0xAA55