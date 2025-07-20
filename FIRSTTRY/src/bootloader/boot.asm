org 0x7C00
bits 16

%define ENDL 0x0D, 0x0A
%define KERNEL_LOAD_SEGMENT 0x2000
%define KERNEL_LOAD_OFFSET 0

jmp short start
nop

; BPB (BIOS Parameter Block) - muss mit mkfs.fat kompatibel sein
bdb_oem:                    db 'MSWIN4.1'           ; 8 bytes
bdb_bytes_per_sector:       dw 512
bdb_sectors_per_cluster:    db 1
bdb_reserved_sectors:       dw 1
bdb_fat_count:              db 2
bdb_dir_entries_count:      dw 224
bdb_total_sectors:          dw 2880                 ; 2880 * 512 = 1.44MB
bdb_media_descriptor_type:  db 0xF0                 ; 3.5" floppy
bdb_sectors_per_fat:        dw 9
bdb_sectors_per_track:      dw 18
bdb_heads:                  dw 2
bdb_hidden_sectors:         dd 0
bdb_large_sector_count:     dd 0

; EBR (Extended Boot Record)
ebr_drive_number:           db 0                    ; 0x00 floppy, 0x80 hdd
ebr_reserved:               db 0
ebr_signature:              db 0x29
ebr_volume_id:              db 0x12, 0x34, 0x56, 0x78    ; serial number, value doesn't matter
ebr_volume_label:           db 'CarlOS     '        ; 11 bytes, padded with spaces
ebr_system_id:              db 'FAT12   '           ; 8 bytes

start:
    cli
    jmp main

; ===================================================================
; Routinen
; ===================================================================

print:
    push si
    push ax
.loop:
    lodsb
    or al, al
    jz .done
    mov ah, 0x0E
    int 0x10
    jmp .loop
.done:
    pop ax
    pop si
    ret

disk_reset:
    pusha
    mov ah, 0
    mov dl, [drive_number]
    int 0x13
    jc floppy_error
    popa
    ret

disk_read:
    ; ax = Start-LBA, cl = Anzahl der Sektoren, bx = Ziel-Offset
    mov [sectors_to_read], cl ; Anzahl der Sektoren für die Retry-Schleife speichern
    mov di, 3                 ; 3 Wiederholungsversuche

.retry:
    pusha                     ; Sichert AX, CX, DX, BX, SP, BP, SI, DI
    
    ; Register für den int 13h Aufruf vorbereiten
    call lba_to_chs           ; Konvertiert LBA in ax zu CHS.
                              ; Setzt ch, cl, dh, dl korrekt für den Interrupt.
    
    mov ah, 0x02              ; BIOS-Funktion: Sektoren lesen
    mov al, [sectors_to_read] ; Anzahl der Sektoren aus der Variable laden
    ; bx, es sind bereits korrekt durch den Aufrufer und pusha/popa
    
    int 0x13
    
    popa                      ; Stellt alle Register wieder her
    
    jnc .done                 ; Wenn erfolgreich (kein Carry), dann fertig

    ; Fehler, erneut versuchen
    call disk_reset
    dec di
    jnz .retry
    jmp floppy_error          ; Alle Versuche fehlgeschlagen

.done:
    ; DEBUG: Zeige ersten Verzeichniseintrag nach erfolgreichem Lesen
    push si
    push cx
    mov si, bx                ; bx zeigt auf den geladenen Puffer
    mov cx, 11
.debug_filename:
    mov al, [si]
    mov ah, 0x0E
    int 0x10
    inc si
    loop .debug_filename
    pop cx
    pop si
    ret

lba_to_chs:
    push dx
    xor dx, dx
    div word [bdb_sectors_per_track]
    inc dl
    mov cl, dl
    xor dx, dx
    div word [bdb_heads]
    mov dh, dl
    mov ch, al
    shl ah, 6
    or cl, ah
    pop dx
    mov dl, [drive_number]
    ret

; ===================================================================
; Hauptprogramm
; ===================================================================

main:
    ; Stack und Segmente korrekt einrichten
    cli                 ; Interrupts deaktivieren, während wir den Stack einrichten
    mov ax, 0           ; Code wird im Segment 0x0000 geladen
    mov ds, ax
    mov es, ax
    mov ss, ax          ; Stack-Segment ebenfalls auf 0x0000 setzen
    mov sp, 0x7C00      ; Stack-Pointer unterhalb des Bootloaders setzen
    sti                 ; Interrupts wieder aktivieren

    mov [drive_number], dl

    ; Erfolgsmeldung drucken
    mov si, hello
    call print

    ; --- Root Directory laden ---
    ; LBA = ReservedSectors + (FatCount * SectorsPerFat)
    mov si, debug_reading_root
    call print
    
    mov ax, [bdb_sectors_per_fat]       ; ax = 9 (Sektoren pro FAT)
    mul byte [bdb_fat_count]            ; ax = ax * 2 (Anzahl der FATs) -> ax = 18
    add ax, [bdb_reserved_sectors]      ; ax = 18 + 1 (Reservierte Sektoren) -> ax = 19
    
    ; Anzahl der Sektoren für das Root-Verzeichnis berechnen
    mov cl, 14
    
    mov bx, buffer                      ; Zieladresse für den Lesevorgang
    call disk_read                      ; Lese Root-Verzeichnis

    ; --- Kernel-Datei im Root Directory suchen ---
    mov si, debug_searching_kernel
    call print

    mov di, buffer                      ; Ziel für den Vergleich setzen (Anfang des Root-Verzeichnisses)
    mov cx, [bdb_dir_entries_count]     ; Anzahl der Einträge im Root-Verzeichnis als Schleifenzähler

.search_loop:
    push cx                             ; Äußeren Schleifenzähler sichern
    
    ; Wir müssen DI für jeden Vergleich zurücksetzen, da repe cmpsb es verändert.
    ; Daher sichern wir es nicht, sondern setzen es relativ zum Buffer-Anfang.
    push di                             ; Aktuelle Position im Buffer sichern für den Vergleich
    
    mov si, file_kernel_bin             ; Quell-String (was wir suchen)
    mov cx, 11                          ; Länge des Dateinamens für den Vergleich
    
    ; repe cmpsb vergleicht [DS:SI] mit [ES:DI]
    repe cmpsb
    
    pop di                              ; Buffer-Position wiederherstellen
    pop cx                              ; Äußeren Schleifenzähler wiederherstellen
    
    je .found_kernel                    ; Wenn Zero-Flag gesetzt ist (Strings sind gleich), dann gefunden!

    add di, 32                          ; Zum nächsten 32-Byte-Eintrag springen
    loop .search_loop                   ; Wiederholen, bis alle Einträge geprüft wurden

    jmp kernel_not_found_error          ; Wenn Schleife endet, wurde nichts gefunden


.found_kernel:
    mov si, debug_found_kernel
    call print
 
    ; Ersten Cluster der Kernel-Datei speichern
    mov ax, [di + 26]
    mov [kernel_cluster], ax

    ; --- FAT laden ---
    mov si, debug_loading_fat
    call print
    
    mov ax, [bdb_reserved_sectors]      ; Start-LBA der FAT (normalerweise 1)
    mov cl, [bdb_sectors_per_fat]       ; Anzahl der Sektoren, die die FAT belegt (normalerweise 9)
    mov bx, buffer                      ; Ziel ist der Puffer
    call disk_read                      ; Lese die FAT in den Puffer

    ; --- Kernel laden ---
    mov bx, KERNEL_LOAD_SEGMENT
    mov es, bx

.load_loop:
    ; Cluster in LBA umwandeln und laden
    mov ax, [kernel_cluster]
    sub ax, 2
    mul byte [bdb_sectors_per_cluster]
    add ax, 33 ; Start des Datenbereichs (Bootsektor + 2 FATs + Root Dir)
    
    mov cl, [bdb_sectors_per_cluster]
    call disk_read
    add bx, [bdb_bytes_per_sector]

    ; Nächsten Cluster aus der FAT lesen
    mov ax, [kernel_cluster]
    mov cx, 3
    mul cx
    mov cx, 2
    div cx
    mov si, buffer
    add si, ax
    mov ax, [ds:si]

    test dx, dx
    jz .even
.odd:
    shr ax, 4
    jmp .next
.even:
    and ax, 0x0FFF
.next:
    cmp ax, 0x0FF8
    jae .done_loading
    mov [kernel_cluster], ax
    jmp .load_loop

.done_loading:
    mov si, debug_loading_kernel
    call print
    
    ; Zum Kernel springen
    mov dl, [drive_number]
    jmp KERNEL_LOAD_SEGMENT:KERNEL_LOAD_OFFSET

; ===================================================================
; Fehlerbehandlung
; ===================================================================

floppy_error:
    mov si, failes_reading
    call print
    jmp halt_system

kernel_not_found_error:
    mov si, fails_finding_kernel
    call print
    jmp halt_system

halt_system:
    hlt
    jmp halt_system

; ===================================================================
; Daten und Variablen
; ===================================================================

hello: db 'CarlOS', ENDL, 0
failes_reading: db 'Disk err!', ENDL, 0
fails_finding_kernel: db 'No kernel!', ENDL, 0
file_kernel_bin: db 'KERNEL  BIN' , ENDL, 0; Nur 2 Leerzeichen zwischen KERNEL und BIN!
debug_loading_kernel: db 'Jump...', ENDL, 0
debug_reading_root: db 'Read root', ENDL, 0
debug_searching_kernel: db 'Find krnl', ENDL, 0
debug_found_kernel: db 'Found', ENDL, 0
debug_loading_fat: db 'Load FAT', ENDL, 0

kernel_cluster: dw 0
sectors_to_read: db 0
drive_number: db 0      

; ===================================================================
; Padding und Boot-Signatur
; ===================================================================

times 510 - ($ - $$) db 0
dw 0xAA55

; ===================================================================
; Puffer (nach der 512-Byte-Grenze)
; ===================================================================
buffer:
    resb 32 * 512 ; 32 Sektoren Puffer für Root Dir und FAT