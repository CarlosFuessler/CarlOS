# Design: Replace Makefile + run.sh with build.zig

## Overview

Replace the current Makefile and run.sh with a single `build.zig` file that orchestrates the entire build and run workflow using Podman containers.

## Current State

- **Makefile**: Compiles C with `x86_64-elf-gcc`, asm with `nasm`, links with `x86_64-elf-ld`, creates ISO with `grub-mkrescue`
- **run.sh**: Creates a Podman container, copies source files, runs make, copies artifacts back, launches QEMU
- **Dockerfile**: Based on `randomdude/gcc-cross-x86_64-elf`, installs nasm, xorriso, grub

## Design

### Files to Create/Modify

1. **`build.zig`** — New file. Main build script.
2. **`build.zig.zon`** — New file. Zig build system package manifest.
3. **`build_env/Dockerfile`** — Modified. Add Zig installation.
4. **`run.sh`** — Deleted. Replaced by build.zig.
5. **`Makefile`** — Deleted. Replaced by build.zig.

### Build Steps

`build.zig` orchestrates the following steps inside a Podman container:

1. **Compile C files** — `zig cc` targeting `x86_64-elf`, freestanding, with `-ffreestanding` equivalent
2. **Compile asm files** — `nasm -f elf64`
3. **Link** — `x86_64-elf-ld -n` with linker script
4. **Create ISO** — `grub-mkrescue`

### Usage

```
zig build          # Build the kernel ISO
zig build start    # Build + run in QEMU
zig build clean    # Remove build artifacts + Podman containers/images
```

### Container Management

- Container name: `carl-os-build`
- Image name: `carl_os_zig`
- `build.zig` creates the container, runs commands inside it, and cleans up
- `zig build clean` runs `podman rm -f` and `podman rmi` for the build image to prevent bloat

### Dockerfile Changes

Add Zig installation (download from official releases, add to PATH). The container will have:
- `zig` (for C compilation)
- `nasm` (for assembly)
- `grub-mkrescue` / `xorriso` (for ISO creation)
- `x86_64-elf-ld` (for linking)

### build.zig Structure

```zig
// Key steps:
// 1. Define the container build step (podman build)
// 2. Define a "build" step that runs inside the container:
//    - zig cc for each C file -> .o files
//    - nasm for each .asm file -> .o files
//    - x86_64-elf-ld to link all .o files -> kernel.bin
//    - grub-mkrescue to create kernel.iso
// 3. Define "start" step that also launches QEMU
// 4. Define "clean" step that removes build dir + containers
```

### QEMU Launch

```bash
qemu-system-x86_64 -cdrom dist/x86_64/kernel.iso -drive file=disk.img,format=raw,index=0,media=disk
```

## Verification

1. Run `zig build` — should produce `dist/x86_64/kernel.iso`
2. Run `zig build start` — should build and launch QEMU with the ISO
3. Run `zig build clean` — should remove build artifacts and clean up Podman resources
4. Verify no orphaned Podman containers remain after clean
