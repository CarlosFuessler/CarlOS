;Bootloader

; Start für linker
global start

extern long_mode_start ; Zeigt auf den c code 

; Titel für Linker
section .text
bits 32 ; 32 bit modus

start:

    mov esp, stack_top ; stack einrichten

    call check_multiboot
    call check_cpuid
    call check_long_mode

    call setup_page_tables
    call enable_paging

    lgdt [gdt64.pointer]
    jmp gdt64.code_segment:long_mode_start

    mov dword [0xb8000], 0x2f4b2f4f
    hlt

check_multiboot:
    cmp eax, 0x36d76289 ; magische nummer für grub
    jne .no_multiboot ; fehler meldung
    ret               ;  weitergabe
.no_multiboot:
    mov al, "M" ;error code
    jmp error  ; springt zur error ausgabe
    
check_cpuid:
    pushfd  ; flags auf stack
    pop eax  ; flags in eax
    mov ecx, eax ; absicherung der flags
    xor eax, 1 << 21  ; id flag umschalten
    push eax ; modifizierte flags auf stack setzen
    popfd  ; neue flags setzen
    
    pushfd  ; Neue flags einlesen
    pop eax  
    push ecx 
    popfd   ; Flags restaurieren

    cmp eax, ecx ; vergleich ob die flags sich verändert haben
    je .no_cpuid ; nein cpu hat keine unterstützung
    ret

.no_cpuid:
    mov al, "C" ; error code
    jmp error
    
check_long_mode:
    mov eax, 0x80000000 ; extended cpuid funktion
    cpuid               
    cmp eax, 0x80000001 ; unterstützung von extended info
    jb .no_long_mode    ; nein kein 64 bit

    mov eax, 0x80000001  ; extended feature info
    cpuid  
    test edx, 1 << 29  ; bit 29 für long mode support
    jz .no_long_mode   ; kein 64 bit
    ret

.no_long_mode:
    mov al, "L"   ; error code
    jmp error

setup_page_tables:
    mov eax, page_table_l3 ; Laden der Level 3 tabellle
    or eax, 0b11 ; read / wirte flag setzten
    mov [page_table_l4], eax ; Eintrag setzten

    mov eax, page_table_l2 ; Lade level 2 Tabelle
    or eax, 0b11 ; read / write flag setzten
    mov [page_table_l3], eax ; Eintrag setzten

    mov ecx, 0 ; Zähler für dei kommende Schleife

.loop: ; Page 2 mit 2MB beschreiben
    mov eax, 0x200000 ; Beschreibt eax mit 2MB
    mul ecx ;multipliziert mit zähler / berchent dei startadresse
    or eax, 0b10000011 ;Setzt Flag für Eintrag read/ write
    mov [page_table_l2 + ecx * 8], eax ; Eintrag in die Page 2

    inc ecx ; Zähler erhöhen
    cmp ecx, 512; ; Prüft ob 512 einträge erstellt wurden
    jne .loop ; lässt den loop laufen bis 512 Eintäge geschrieben wurden

    ret

    ; Mit dem Loop umgehen wir das nutzen der Page 1 und zeigen direkt auf den Physischen speicher
    ; Sowie beschreiben wir 1 gigabyte auf den physischen speicher was wichtig für den Übergang in den Longmode ist sowie für das Kernel
enable_paging: ; Longmode aktivieren

    mov eax, page_table_l4 ; Adresse von Page 4 in eax
    mov cr3, eax    ; eax auf cr3

    mov eax, cr4 ; cr4 lesen
    or eax, 1 << 5 <; mit Bit 5 longmode erzwimgen
    mov cr4, eax ; reset

    mov ecx, 0xC0000080
    rdmsr
    or eax, 1 << 8 ; Aktiviert den Longmode logisch durch Bit 8
    wrmsr ;reset

    mov eax, cr0 ; cr0 lesen
    or eax, 1 << 31 ; Paging durch Bit 31 aktivieren
    mov cr0, eax ;reset

    ret 

error: ; error handeling
    ;0xb8000 ist der VGA-Textmodus
    mov dword [0xb8000], 0x4f524f45 
    mov dword [0xb8004], 0x4f3a4f52
    mov dword [0xb8008], 0x4f204f20
    mov dword [0xb800c], 0x4f204f20
    mov [0xb8010], al
    hlt  

section .bss

align 4096 ;alles was nicht beschrieben ist mit 0 fllen

;Größe der Pages bestimmen

page_table_l4:
    resb 4096

page_table_l3:
    resb 4096

page_table_l2:
    resb 4096
          
; 64 KiB Stack einrichten wächst nach unten         
          
stack_bottom:
    resb 4096 * 16


stack_top: ;Startwert

section .rodata

gdt64:

    dq 0
.code_segment: equ $ - gdt64    
    dq (1 << 43) | (1 << 44) | (1 << 47) | (1 << 53)
      ; 64‑Bit Code‑Segment:
      ; Bit47 P=1 Present
      ; Bit44 S=1 Code/Data
      ; Bit43 Exec=1 Code
      ; Bit53 L=1 64‑Bit

.pointer: 
    dw $ - gdt64 - 1 ; Limit
    dq gdt64 ; Basis für gdt