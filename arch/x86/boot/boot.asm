org 0x7c00
bits 16

%define KERNEL_LOAD_SEGMENT 0x1000
%define KERNEL_ENTRY_POINT  0x100000
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
    ; Calculate total sectors to load: kernel + ramdisk
    mov ax, KERNEL_LOAD_SECTORS
    add ax, RAMDISK_LOAD_SECTORS
    mov [dap.sectors], ax

    ; check if LBA is supported
    mov ah, 0x41
    mov bx, 0x55AA
    int 0x13
    jc .no_lba
    cmp bx, 0xAA55
    jne .no_lba

    ; use LBA to read kernel + ramdisk payload immediately after the boot sector
    mov ah, 0x42
    mov dl, [boot_drive]
    mov si, dap
    int 0x13
    jc disk_error
    jmp protected_mode_entry

.no_lba:
    ; CHS fallback: read one sector (works for tiny kernels on old BIOSes)
    mov ah, 0x02
    mov al, byte [dap.sectors]
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

    ; Kernel is read in real mode to 0x10000; move it to 0x100000.
    ; Use byte count for accuracy
    mov esi, 0x00010000
    mov edi, KERNEL_ENTRY_POINT
    mov ecx, KERNEL_SIZE_BYTES
    add ecx, 3                  ; Round up to nearest dword
    shr ecx, 2                  ; Convert to dword count
    rep movsd

    ; Ramdisk is after kernel in the loaded buffer; move it to 0x200000.
    ; Calculate ramdisk source: (kernel buffer + KERNEL_SIZE_BYTES)
    mov esi, 0x00010000
    add esi, KERNEL_SIZE_BYTES  ; ramdisk starts here in the buffer
    mov edi, 0x00200000         ; moving to 0x200000
    mov ecx, RAMDISK_SIZE_BYTES
    add ecx, 3                  ; Round up to nearest dword
    shr ecx, 2                  ; Convert to dword count
    rep movsd

    jmp KERNEL_ENTRY_POINT

disk_error:
    cli
disk_hang:
    hlt
    jmp disk_hang

boot_drive: db 0

dap:
    .size:    db 0x10                 ; size of DAP
    .reserved: db 0                    ; reserved
    .sectors: dw KERNEL_LOAD_SECTORS  ; sectors to read (will be updated in load_kernel)
    .offset:  dw 0x0000               ; buffer offset
    .segment: dw KERNEL_LOAD_SEGMENT  ; buffer segment (loads to 0x10000)
    .lba:     dq 1                    ; LBA start sector

times 510 - ($ - $$) db 0 ; padding
dw 0xAA55