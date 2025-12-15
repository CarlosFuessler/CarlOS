; Multiboot header (for compatibility)

; For the linker
section .multiboot_header

; Start label
header_start:

; Magic number for Multiboot
    dd 0xe85250d6

; Architecture

    dd 0

; Header length

    dd header_end -header_start

    dd 0x100000000 - (0xe85250d6 + 0 + header_end -header_start)

    dw 0
    dw 0
    dd 8

; End of header
header_end: