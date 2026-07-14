# CarlOS Assembly Code Dokumentation

## Überblick

Das CarlOS-Betriebssystem verwendet drei Haupt-Assembly-Dateien für den Boot-Prozess und den Übergang vom 32-Bit- zum 64-Bit-Modus:

1. **header.asm** - Multiboot-Header für GRUB
2. **main.asm** - 32-Bit Bootloader mit System-Checks
3. **main64.asm** - 64-Bit Long Mode Einstiegspunkt

---

## Dateistruktur

```
src/main/x86_64/
├── header.asm          # Multiboot2-Header
└── boot/
    ├── main.asm        # 32-Bit Bootloader
    └── main64.asm      # 64-Bit Initialisierung
```

---

## 1. header.asm - Multiboot-Header

### Zweck
Diese Datei definiert den Multiboot2-Header, der benötigt wird, damit GRUB (der Bootloader) das Betriebssystem laden kann.

### Code-Erklärung

```asm
section .multiboot_header
```
- Definiert eine spezielle Section für den Multiboot-Header
- Diese Section wird vom Linker am Anfang des Binaries platziert

```asm
dd 0xe85250d6
```
- **Magic Number** für Multiboot2-Spezifikation
- GRUB sucht nach dieser Zahl, um ein gültiges OS-Image zu erkennen

```asm
dd 0
```
- **Architektur-Feld**: 0 = i386 (32-Bit geschützt)

```asm
dd header_end - header_start
```
- **Header-Länge**: Berechnet die Größe des Headers dynamisch

```asm
dd 0x100000000 - (0xe85250d6 + 0 + header_end - header_start)
```
- **Checksum**: Validierung des Headers
- Magic + Architektur + Länge + Checksum = 0 (32-Bit Arithmetik)

```asm
dw 0
dw 0
dd 8
```
- **End-Tag**: Markiert das Ende des Multiboot-Headers

---

## 2. main.asm - 32-Bit Bootloader

### Überblick
Dies ist die Hauptdatei des Bootloaders. Sie führt Systemprüfungen durch, richtet Paging ein und wechselt in den 64-Bit Long Mode.

### Boot-Ablauf

```
1. Stack initialisieren
2. Multiboot-Check
3. CPUID-Unterstützung prüfen
4. Long Mode-Unterstützung prüfen
5. Page Tables einrichten
6. Paging aktivieren
7. In 64-Bit Long Mode wechseln
```

### Detaillierte Code-Analyse

#### Start-Routine

```asm
global start
extern long_mode_start

section .text
bits 32
```
- `global start`: Macht das Label `start` für den Linker sichtbar
- `extern long_mode_start`: Referenziert die 64-Bit-Einstiegsfunktion
- `bits 32`: Teilt NASM mit, dass wir 32-Bit-Code generieren

```asm
start:
    mov esp, stack_top
```
- Setzt den Stack Pointer auf den Anfang des Stacks (der Stack wächst nach unten)
- Essentiell für Function Calls und lokale Variablen

#### 1. Multiboot-Check

```asm
check_multiboot:
    cmp eax, 0x36d76289
    jne .no_multiboot
    ret
.no_multiboot:
    mov al, "M"
    jmp error
```
- GRUB lädt `0x36d76289` in EAX beim Booten
- Wenn diese Magic Number nicht vorhanden ist → Fehler
- Fehlercode: "M" für Multiboot-Fehler

#### 2. CPUID-Check

```asm
check_cpuid:
    pushfd              # FLAGS-Register auf Stack speichern
    pop eax             # FLAGS nach EAX kopieren
    mov ecx, eax        # Kopie in ECX behalten
    xor eax, 1 << 21    # ID-Flag (Bit 21) umschalten
    push eax            
    popfd               # Modifizierte FLAGS zurückschreiben
    
    pushfd              # FLAGS erneut lesen
    pop eax  
    push ecx 
    popfd               # Ursprüngliche FLAGS wiederherstellen

    cmp eax, ecx        # Hat sich Bit 21 geändert?
    je .no_cpuid        # Nein → keine CPUID-Unterstützung
    ret
```
- **CPUID** ist eine Instruktion zur Abfrage von CPU-Features
- Wenn Bit 21 des FLAGS-Registers änderbar ist, wird CPUID unterstützt
- Fehlercode: "C" für CPUID-Fehler

#### 3. Long Mode Check

```asm
check_long_mode:
    mov eax, 0x80000000
    cpuid               
    cmp eax, 0x80000001
    jb .no_long_mode

    mov eax, 0x80000001
    cpuid  
    test edx, 1 << 29
    jz .no_long_mode
    ret
```
- Prüft, ob die CPU 64-Bit (Long Mode) unterstützt
- `0x80000000`: Maximale erweiterte CPUID-Funktion abfragen
- `0x80000001`: Erweiterte CPU-Features abfragen
- Bit 29 in EDX: Long Mode-Flag
- Fehlercode: "L" für Long Mode-Fehler

#### 4. Page Tables Einrichten

```asm
setup_page_tables:
    mov eax, page_table_l3
    or eax, 0b11                # Present + Writable
    mov [page_table_l4], eax
    
    mov eax, page_table_l2
    or eax, 0b11
    mov [page_table_l3], eax
    
    mov ecx, 0
.loop:
    mov eax, 0x200000           # 2MB
    mul ecx
    or eax, 0b10000011          # Present + Writable + Huge Page
    mov [page_table_l2 + ecx * 8], eax
    
    inc ecx
    cmp ecx, 512
    jne .loop
    ret
```

**Page Table Hierarchie:**
```
Level 4 (PML4) → Level 3 (PDPT) → Level 2 (PD) → Physischer Speicher
    1 Eintrag        1 Eintrag       512 Einträge (je 2MB)
```

- **Identity Mapping**: Virtuelle Adressen = Physische Adressen
- **Huge Pages**: 2MB-Seiten statt 4KB (effizienter)
- **512 × 2MB = 1GB** gemappter Speicher
- Flags:
  - Bit 0: **Present** (Seite ist gültig)
  - Bit 1: **Writable** (Schreibzugriff erlaubt)
  - Bit 7: **Huge Page** (2MB statt 4KB)

#### 5. Paging Aktivieren

```asm
enable_paging:
    mov eax, page_table_l4
    mov cr3, eax                # CR3 enthält Page Table-Adresse
    
    mov eax, cr4
    or eax, 1 << 5              # PAE (Physical Address Extension)
    mov cr4, eax
    
    mov ecx, 0xC0000080         # IA32_EFER MSR
    rdmsr
    or eax, 1 << 8              # LME (Long Mode Enable)
    wrmsr
    
    mov eax, cr0
    or eax, 1 << 31             # PG (Paging Enable)
    mov cr0, eax
    ret
```

**Aktivierungsschritte:**
1. **CR3**: Lädt die Adresse der Level-4 Page Table
2. **PAE**: Physical Address Extension aktivieren (für >4GB RAM)
3. **LME**: Long Mode Enable im EFER-Register setzen
4. **PG**: Paging aktivieren → Long Mode ist aktiv!

#### 6. Wechsel zu 64-Bit

```asm
lgdt [gdt64.pointer]
jmp gdt64.code_segment:long_mode_start
```
- Lädt die Global Descriptor Table (GDT)
- Far Jump zum 64-Bit Code Segment
- Nach diesem Jump sind wir im 64-Bit-Modus!

#### Fehlerbehandlung

```asm
error:
    mov dword [0xb8000], 0x4f524f45  # "ER"
    mov dword [0xb8004], 0x4f3a4f52  # "R:"
    mov dword [0xb8008], 0x4f204f20  # "  "
    mov dword [0xb800c], 0x4f204f20  # "  "
    mov [0xb8010], al                # Fehlercode
    hlt
```
- **0xb8000**: VGA Text Mode Speicher
- Format: `0x4fXX` = weißer Text (0x0f) auf rotem Hintergrund (0x04)
- Zeigt "ERR: X" auf dem Bildschirm (X = M/C/L)

### Speicherlayout (.bss Section)

```asm
section .bss
align 4096

page_table_l4:
    resb 4096           # 4KB für Level 4 Table

page_table_l3:
    resb 4096           # 4KB für Level 3 Table

page_table_l2:
    resb 4096           # 4KB für Level 2 Table

stack_bottom:
    resb 4096 * 16      # 64KB Stack

stack_top:
```
- `.bss`: Uninitialisierter Speicher (mit Nullen gefüllt)
- `align 4096`: Page Tables müssen 4KB-aligned sein
- Stack: 64KB groß, wächst von `stack_top` nach unten

### Global Descriptor Table (GDT)

```asm
section .rodata

gdt64:
    dq 0                                    # Null-Descriptor
.code_segment: equ $ - gdt64    
    dq (1 << 43) | (1 << 44) | (1 << 47) | (1 << 53)

.pointer: 
    dw $ - gdt64 - 1                        # Limit
    dq gdt64                                # Base-Adresse
```

**GDT-Flags Erklärung:**
- Bit 43 (Exec): Ausführbares Code-Segment
- Bit 44 (S): Code/Data-Segment (nicht System)
- Bit 47 (P): Present (Segment ist gültig)
- Bit 53 (L): Long Mode (64-Bit Code)

Im Long Mode werden Segmentgrenzen ignoriert, aber die GDT wird trotzdem benötigt!

---

## 3. main64.asm - 64-Bit Initialisierung

### Zweck
Diese Datei wird aufgerufen, nachdem der Prozessor in den 64-Bit-Modus gewechselt ist.

### Code-Erklärung

```asm
global long_mode_start
extern kernel_main

section .text
bits 64
```
- `bits 64`: Ab jetzt 64-Bit-Instruktionen
- `extern kernel_main`: Referenz zur C-Kernel-Funktion

```asm
long_mode_start:
    mov ax, 0
    mov ss, ax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
```
- Setzt alle Segment-Register auf 0
- Im Long Mode sind Segmente nicht mehr wichtig (Flat Memory Model)
- Trotzdem müssen sie mit gültigen Werten initialisiert werden

```asm
    call kernel_main
    hlt
```
- Springt zum C-Kernel (`main.c`)
- Falls `kernel_main` zurückkehrt: CPU anhalten

---

## Boot-Prozess Gesamtübersicht

```
┌─────────────────────────────────────────────────────────────┐
│ 1. BIOS/UEFI lädt GRUB                                      │
└────────────────────┬────────────────────────────────────────┘
                     │
┌────────────────────▼────────────────────────────────────────┐
│ 2. GRUB findet Multiboot-Header (header.asm)               │
│    und lädt das OS in den Speicher                         │
└────────────────────┬────────────────────────────────────────┘
                     │
┌────────────────────▼────────────────────────────────────────┐
│ 3. GRUB springt zu 'start' in main.asm (32-Bit)           │
└────────────────────┬────────────────────────────────────────┘
                     │
┌────────────────────▼────────────────────────────────────────┐
│ 4. System-Checks:                                           │
│    ✓ Multiboot-Magic                                       │
│    ✓ CPUID-Unterstützung                                   │
│    ✓ Long Mode-Fähigkeit                                   │
└────────────────────┬────────────────────────────────────────┘
                     │
┌────────────────────▼────────────────────────────────────────┐
│ 5. Paging einrichten:                                       │
│    • Page Tables erstellen                                  │
│    • 1GB Identity Mapping (2MB Pages)                      │
│    • PAE + Long Mode aktivieren                            │
│    • Paging einschalten                                     │
└────────────────────┬────────────────────────────────────────┘
                     │
┌────────────────────▼────────────────────────────────────────┐
│ 6. GDT laden und zu 64-Bit wechseln                        │
│    jmp gdt64.code_segment:long_mode_start                  │
└────────────────────┬────────────────────────────────────────┘
                     │
┌────────────────────▼────────────────────────────────────────┐
│ 7. main64.asm: Segment-Register initialisieren (64-Bit)   │
└────────────────────┬────────────────────────────────────────┘
                     │
┌────────────────────▼────────────────────────────────────────┐
│ 8. Sprung zu kernel_main() in C (main.c)                  │
│    → Das Betriebssystem läuft! 🎉                          │
└─────────────────────────────────────────────────────────────┘
```

---

## Speicher-Layout während des Boots

```
0x00000000 ┌──────────────────────────────────┐
           │ Real Mode IVT (Interrupt Table) │
0x00007C00 ├──────────────────────────────────┤
           │ GRUB Bootloader                  │
0x00100000 ├──────────────────────────────────┤
           │ Kernel Code + Data               │
           │ - header.asm (Multiboot)        │
           │ - main.asm (Bootloader)         │
           │ - main64.asm (Long Mode)        │
           │ - kernel_main (C Code)          │
           ├──────────────────────────────────┤
           │ BSS Section:                     │
           │ - Page Tables (L4, L3, L2)      │
           │ - Stack (64KB)                  │
           ├──────────────────────────────────┤
0x000B8000 │ VGA Text Buffer                  │
0x000C0000 ├──────────────────────────────────┤
           │ Extended Memory                  │
           │ (für OS-Nutzung)                │
           └──────────────────────────────────┘
```

---

## Wichtige Konzepte

### Protected Mode vs. Long Mode

| Feature | Protected Mode (32-Bit) | Long Mode (64-Bit) |
|---------|------------------------|-------------------|
| Register | EAX, EBX, ECX, ... | RAX, RBX, RCX, ... |
| Adressraum | 4 GB (2³²) | 16 EB (2⁶⁴ theoretisch) |
| Pointer-Größe | 4 Bytes | 8 Bytes |
| Segmentierung | Aktiv | Flat Model |
| Paging | Optional | Verpflichtend |

### Paging-Hierarchie

```
Virtuelle Adresse (64-Bit, nur 48 Bit genutzt):
┌──────────┬──────────┬──────────┬──────────┬───────────────────┐
│ 16 Bit   │ 9 Bit    │ 9 Bit    │ 9 Bit    │ 21 Bit            │
│ Ignored  │ L4 Index │ L3 Index │ L2 Index │ Page Offset (2MB) │
└──────────┴──────────┴──────────┴──────────┴───────────────────┘
     ▲          │          │          │              │
     │          ▼          ▼          ▼              │
     │        ┌────┐    ┌────┐    ┌────┐            │
     │   CR3→ │ L4 │ ─→ │ L3 │ ─→ │ L2 │ ──────────┤
     │        └────┘    └────┘    └────┘            │
     │                                               ▼
     │                                        ┌──────────────┐
     └────────────────────────────────────────│ Phys. Memory│
                                              └──────────────┘
```

### CPU-Register im x86_64

**General Purpose Register:**
- RAX, RBX, RCX, RDX (64-Bit)
- RSI, RDI, RSP, RBP
- R8 - R15 (nur 64-Bit)

**Control Registers:**
- CR0: System-Kontrolle (Paging Enable, etc.)
- CR3: Page Table Base Address
- CR4: Extended Features (PAE, etc.)

**Segment Registers:**
- CS, DS, ES, FS, GS, SS

**Special Registers:**
- RIP: Instruction Pointer
- RFLAGS: Status Flags
- MSRs: Model-Specific Registers (z.B. EFER)

---

## Fehlercodes

| Code | Bedeutung | Ursache |
|------|-----------|---------|
| **M** | Multiboot-Fehler | OS wurde nicht von GRUB geladen |
| **C** | CPUID-Fehler | CPU zu alt (vor Pentium) |
| **L** | Long Mode-Fehler | CPU unterstützt kein 64-Bit |

---

## Verbindung zu C-Code

Das Assembly-Framework bereitet alles für den C-Kernel vor:

```c
// main.c
void kernel_main(void) {
    // An diesem Punkt:
    // ✓ 64-Bit Long Mode aktiv
    // ✓ Paging eingerichtet (1GB gemappt)
    // ✓ Stack verfügbar (64KB)
    // ✓ Segment-Register initialisiert
    
    // Kernel-Code kann ausgeführt werden!
}
```

Die Linkage erfolgt über [linker.ld](targets/x86_64/linker.ld), der alle Sections zusammenfügt.

---

## Build-Prozess

```bash
# Assembly → Object Files
nasm -f elf64 header.asm -o header.o
nasm -f elf64 main.asm -o main.o
nasm -f elf64 main64.asm -o main64.o

# C Code → Object File
gcc -c main.c -o main.o

# Linken zu bootbarem Kernel
ld -n -o kernel.bin -T linker.ld *.o

# ISO-Image erstellen (bootbar)
grub-mkrescue -o os.iso iso/
```

---

## Nützliche Ressourcen

- **OSDev Wiki**: https://wiki.osdev.org/
- **Intel SDM**: Intel Software Developer Manuals
- **Multiboot2 Spec**: https://www.gnu.org/software/grub/manual/multiboot2/
- **x86 Instruction Reference**: https://www.felixcloutier.com/x86/

---

## Zusammenfassung

Die Assembly-Dateien von CarlOS bilden die kritische Brücke zwischen dem Bootloader (GRUB) und dem C-Kernel:

1. **header.asm** macht das OS für GRUB erkennbar
2. **main.asm** prüft das System, richtet Paging ein und aktiviert 64-Bit
3. **main64.asm** initialisiert den 64-Bit-Modus und startet den C-Kernel

Dieser modulare Aufbau trennt Hardware-Initialisierung (ASM) von der eigentlichen OS-Logik (C), was die Wartbarkeit und Portabilität verbessert.
