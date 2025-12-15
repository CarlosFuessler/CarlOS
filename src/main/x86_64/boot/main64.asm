; Switch from 32-bit to 64-bit
global long_mode_start
extern kernel_main

; For the linker
section .text
bits 64


long_mode_start:
    ; load 0 into all data segment registers
    mov ax, 0
    mov ss, ax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    ; call the kernel (main.c)
    call kernel_main

    hlt

; ASM and C are linked via linker.ld