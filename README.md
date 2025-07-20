# CarlOS

Ein einfaches Betriebssystem geschrieben in Assembly und C.

## Build-Anweisungen

```bash
# Docker-Container starten
docker run --rm -it -v $(pwd):/root/env carl_os

# Projekt kompilieren
make build-x86_64

# Um das Projekt zu starten 
qemu-system-x86_64 -cdrom  -fda dist/x86_64/kernel.iso
```

## Projektstruktur

- `src/main/x86_64/boot/` - Boot-Assembly-Code
- `src/main/kernel/` - Kernel-C-Code
- `src/main/interface/` - Header-Dateien
- `targets/x86_64/` - Linker-Scripts und ISO-Konfiguration

## Features

- 32-bit zu 64-bit Long Mode Übergang
- VGA-Text-Modus-Ausgabe
- Grundlegende Print-Funktionen
