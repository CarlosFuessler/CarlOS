ENTRY(start)

SECTIONS
{
  . = 1M;

  .multiboot :
  {
    KEEP(*(.multiboot_header))
  }

  .text :
  {
    *(.text*)
  }

  .rodata :
  {
    *(.rodata*)
  }

  .data :
  {
    *(.data*)
  }

  .bss :
  {
    *(COMMON)
    *(.bss*)
    *(.sbss*)
  }
}