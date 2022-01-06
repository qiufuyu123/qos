ENTRY(loader)                /* the name of the entry label */

SECTIONS {
    . = 0xC0100000;                  /* the code should be loaded at 3GB + 1 MB */

    .text : AT(ADDR(.text) - 0xC0000000)
    {
        *(.multiboot)
        *(.text)
        *(.rodata*)
    }

    .data ALIGN (0x1000) : AT(ADDR(.data) - 0xC0000000)
    {
        *(.data)
    }

    .bss : AT(ADDR(.bss) - 0xC0000000)
    {
        *(COMMON)
        *(.bss)
    }
    kernel_virt_end_var = .;
    kernel_phys_end_var = (. - 0xC0000000);
}