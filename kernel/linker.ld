ENTRY(_start)

SECTIONS
{
    . = 0x00010000;

    .text : {
        build/boot/kernel_entry.o(.text)
        *(.text*)
    }

    .rodata : {
        *(.rodata*)
    }

    .data : {
        *(.data*)
    }

    .bss : {
        *(COMMON)
        *(.bss*)
    }
}
