; Bootloader

; Entry point for the linker
global start

extern long_mode_start ; points to the C code

; Code section for the linker
section .text
bits 32 ; 32-bit mode

start:

    mov esp, stack_top ; set up stack

    call check_multiboot
    call check_cpuid
    call check_long_mode

    call setup_page_tables
    call enable_paging

    lgdt [gdt64.pointer]
    jmp gdt64.code_segment:long_mode_start

    mov dword [0xb8000], 0x2f4b2f4f ; write to VGA (debug)
    hlt

check_multiboot:
    cmp eax, 0x36d76289 ; GRUB magic number
    jne .no_multiboot   ; error
    ret                 ; ok
.no_multiboot:
    mov al, "M" ; error code
    jmp error    ; jump to error output
    
check_cpuid:
    pushfd            ; save flags
    pop eax           ; flags -> eax
    mov ecx, eax      ; keep a copy
    xor eax, 1 << 21  ; toggle ID flag
    push eax          ; write modified flags
    popfd             ; set new flags
    
    pushfd            ; read flags again
    pop eax  
    push ecx 
    popfd             ; restore original flags

    cmp eax, ecx ; did flags change?
    je .no_cpuid ; no -> no CPUID support
    ret

.no_cpuid:
    mov al, "C" ; error code
    jmp error
    
check_long_mode:
    mov eax, 0x80000000 ; extended CPUID function
    cpuid               
    cmp eax, 0x80000001 ; extended info supported?
    jb .no_long_mode    ; no -> not 64-bit capable

    mov eax, 0x80000001  ; extended feature info
    cpuid  
    test edx, 1 << 29  ; bit 29 = long mode support
    jz .no_long_mode   ; no 64-bit
    ret

.no_long_mode:
    mov al, "L"   ; error code
    jmp error

setup_page_tables:
    mov eax, page_table_l3      ; load level 3 table
    or eax, 0b11                ; present + writable
    mov [page_table_l4], eax    ; L4[0] -> L3

    mov eax, page_table_l2      ; load level 2 table
    or eax, 0b11                ; present + writable
    mov [page_table_l3], eax    ; L3[0] -> L2

    mov ecx, 0 ; loop counter

.loop: ; fill L2 with 2MB pages
    mov eax, 0x200000 ; 2MB
    mul ecx           ; eax = 2MB * index
    or eax, 0b10000011 ; present + writable + 2MB page
    mov [page_table_l2 + ecx * 8], eax ; write L2 entry

    inc ecx ; next entry
    cmp ecx, 512; ; check 512 entries
    jne .loop ; loop until done

    ret

    ; This avoids using L1 and maps physical memory directly.
    ; 512 * 2MB = 1GB identity mapping for long mode + kernel.
enable_paging: ; enable paging + long mode

    mov eax, page_table_l4 ; address of L4 in eax
    mov cr3, eax           ; load CR3

    mov eax, cr4          ; read CR4
    or eax, 1 << 5; enable PAE (needed for long mode)
    mov cr4, eax          ; write back

    mov ecx, 0xC0000080
    rdmsr
    or eax, 1 << 8 ; set LME (Long Mode Enable)
    wrmsr          ; write back

    mov eax, cr0      ; read CR0
    or eax, 1 << 31   ; enable paging (PG)
    mov cr0, eax      ; write back

    ret 

error: ; error handling
    ; 0xb8000 is VGA text mode memory
    mov dword [0xb8000], 0x4f524f45 
    mov dword [0xb8004], 0x4f3a4f52
    mov dword [0xb8008], 0x4f204f20
    mov dword [0xb800c], 0x4f204f20
    mov [0xb8010], al ; print error letter
    hlt  

section .bss

align 4096 ; align to page size (fill with zeros)

; Page table storage (each table is 4096 bytes)

page_table_l4:
    resb 4096

page_table_l3:
    resb 4096

page_table_l2:
    resb 4096
          
; 64 KiB stack (grows downward)
          
stack_bottom:
    resb 4096 * 16


stack_top: ; initial stack pointer

section .rodata

gdt64:

    dq 0
.code_segment: equ $ - gdt64    
    dq (1 << 43) | (1 << 44) | (1 << 47) | (1 << 53)
      ; 64‑Bit Code‑Segment:
    ; 64-bit code segment:
    ; P=1 present, S=1 code/data, Exec=1 code, L=1 64-bit

.pointer: 
    dw $ - gdt64 - 1 ; limit
    dq gdt64         ; base