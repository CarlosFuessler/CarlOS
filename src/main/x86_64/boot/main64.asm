; Switch von 32 Bit aif 64 Bit
global long_mode_start
extern kernel_main

; Für Linker 
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

    ; ruft das Kernel auf (main.c)
    call kernel_main

    hlt

; Verbindung von ASM Datein und C Datein funktioniert über die linker.ld 