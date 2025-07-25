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
    jz .no_long_mode   ; kein 65 bit
    ret

.no_long_mode:
    mov al, "L"   ; error code
    jmp error

setup_page_tables:
    mov eax, page_table_l3
    or eax, 0b11
    mov [page_table_l4], eax

    mov eax, page_table_l2
    or eax, 0b11
    mov [page_table_l3], eax

    mov ecx, 0

.loop:
    mov eax, 0x200000
    mul ecx
    or eax, 0b10000011
    mov [page_table_l2 + ecx * 8], eax

    inc ecx
    cmp ecx, 512
    jne .loop

    ret

enable_paging:
    mov eax, page_table_l4
    mov cr3, eax

    mov eax, cr4
    or eax, 1 << 5
    mov cr4, eax

    mov ecx, 0xC0000080
    rdmsr
    or eax, 1 << 8
    wrmsr

    mov eax, cr0
    or eax, 1 << 31
    mov cr0, eax

    ret

error:
    mov dword [0xb8000], 0x4f524f45 
    mov dword [0xb8004], 0x4f3a4f52
    mov dword [0xb8008], 0x4f204f20
    mov dword [0xb800c], 0x4f204f20
    mov [0xb8010], al
    hlt  

section .bss

align 4096

page_table_l4:
    resb 4096

page_table_l3:
    resb 4096

page_table_l2:
    resb 4096
          
stack_bottom:
    resb 4096 * 4

stack_top:

section .rodata

gdt64:

    dq 0
.code_segment: equ $ - gdt64    
    dq (1 << 43) | (1 << 44) | (1 << 47) | (1 << 53)

.pointer:
    dw $ - gdt64 - 1
    dq gdt64