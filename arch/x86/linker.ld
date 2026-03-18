ENTRY(_start)

PHDRS
{
    text PT_LOAD FLAGS(5); /* R-X */
    data PT_LOAD FLAGS(6); /* RW- */
}

SECTIONS
{
    . = 0x00100000;
    _kernel_start = .;
    .text : {
        build/boot/kernel_entry.o(.text)
        *(.text*)
    } :text

    .rodata : {
        *(.rodata*)
    } :text

    .data : {
        *(.data*)
    } :data

    .bss : {
        *(COMMON)
        *(.bss*)
    } :data

    . = ALIGN(4096);
    _kernel_end = .;

    . = 0x00200000;
    _ramdisk_start = .;
    . = 0x00900000;
    _ramdisk_end = .;
    _heap_start = .;
}
