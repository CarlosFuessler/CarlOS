org 0   
bits 16

%define ENDL 0x0D, 0x0A

start:
    ; Segmente initialisieren
    mov ax, cs
    mov ds, ax      ; Datensegment setzen
    mov es, ax      ; Extra-Segment setzen
    mov ss, ax      ; Stack-Segment setzen
    mov sp, 0xFFFE  ; Stack-Pointer setzen
    
    ; Nachricht ausgeben
    mov si, hello
    call print
    
    ; System anhalten
.halt:
    hlt
    jmp .halt

; Hilfsfunktion zum Ausgeben von Text
print:
    pusha
.loop:
    lodsb           ; Lade das nächste Zeichen
    or al, al       ; Teste auf Null-Terminator
    jz .done        ; Wenn Null, dann fertig
    mov ah, 0x0E    ; BIOS-Teletype-Funktion
    int 0x10        ; BIOS-Interrupt aufrufen
    jmp .loop
.done:
    popa
    ret

; Daten
hello: db 'Hello from CarlOS Kernel!', ENDL, 0