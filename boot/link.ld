ENTRY(loader)                /* the name of the entry label */

SECTIONS {
    . = 0xC0100000;                  /* the code should be loaded at 3GB + 1 MB */

    .text : AT(ADDR(.text) - 0xC0000000)
    {
        *(.multiboot)
        code = .; _code = .; __code = .;
        *(.text)
        . = ALIGN(4096);
    }

    .data ALIGN(0x1000): AT(ADDR(.data) - 0xC0000000)
    {
     data = .; _data = .; __data = .;
     *(.data)
     *(.rodata)
     . = ALIGN(4096);
    }

    .bss  : AT(ADDR(.bss) - 0xC0000000)
    {
    bss = .; _bss = .; __bss = .;
    *(.bss)
    . = ALIGN(4096);
    }
    kernel_virt_end_var = .;
    kernel_phys_end_var = (. - 0xC0000000);
}