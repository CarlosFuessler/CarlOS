; Multiboot Header trägt zur kompatiblität

; Für den Linker
section .multiboot_header

; Start Label
header_start:

;Magische Nummer wichtig für den Multiboot Header
    dd 0xe85250d6

;Architektur

    dd 0

;Länge des Headers

    dd header_end -header_start

    dd 0x100000000 - (0xe85250d6 + 0 + header_end -header_start)

    dw 0
    dw 0
    dd 8

; Ende des Headers    
header_end: