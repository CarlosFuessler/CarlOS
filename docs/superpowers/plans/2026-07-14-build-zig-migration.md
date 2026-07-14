# CarlOS build.zig Migration Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Replace Makefile + run.sh with a single build.zig that orchestrates the build using Podman containers.

**Architecture:** build.zig runs on the host, invokes Podman to build inside a container with Zig, nasm, and grub. The container image is rebuilt from an updated Dockerfile that includes Zig.

**Tech Stack:** Zig build system, Podman, nasm, x86_64-elf-ld, grub-mkrescue

## Global Constraints

- Container-based build (Podman)
- Cross-compilation target: x86_64-elf
- Must produce bootable ISO via GRUB multiboot2
- Assembly files use NASM (elf64 format)
- Linker script at `targets/x86_64/linker.ld`

---

## File Structure

| File | Action | Purpose |
|------|--------|---------|
| `build.zig.zon` | Create | Zig package manifest |
| `build.zig` | Create | Main build script |
| `build_env/Dockerfile` | Modify | Add Zig installation |
| `Makefile` | Delete | Replaced by build.zig |
| `run.sh` | Delete | Replaced by build.zig |
| `.gitignore` | Modify | Add Zig build artifacts |

---

### Task 1: Create build.zig.zon

**Files:**
- Create: `build.zig.zon`

**Interfaces:**
- Produces: Package manifest consumed by `build.zig`

- [ ] **Step 1: Create build.zig.zon**

```zon
.{
    .name = "carlos",
    .version = "0.1.0",
    .minimum_zig_version = "0.13.0",
    .paths = .{
        "build.zig",
        "build.zig.zon",
        "src",
        "targets",
    },
}
```

- [ ] **Step 2: Verify file is valid**

Run: `zig build --help`
Expected: Shows help text (no build steps defined yet, but manifest parses)

- [ ] **Step 3: Commit**

```bash
git add build.zig.zon
git commit -m "build: add build.zig.zon manifest"
```

---

### Task 2: Update Dockerfile to include Zig

**Files:**
- Modify: `build_env/Dockerfile`

**Interfaces:**
- Produces: Docker image `carl_os_zig` with Zig, nasm, grub, xorriso

- [ ] **Step 1: Update Dockerfile**

```dockerfile
FROM debian:bookworm-slim

RUN apt-get update && \
    apt-get upgrade -y && \
    apt-get install -y \
        nasm \
        xorriso \
        grub-common \
        grub-pc-bin \
        wget \
        xz-utils \
        && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

# Install Zig
RUN wget -q https://ziglang.org/download/0.13.0/zig-linux-x86_64-0.13.0.tar.xz -O /tmp/zig.tar.xz && \
    tar -xf /tmp/zig.tar.xz -C /usr/local && \
    mv /usr/local/zig-linux-x86_64-0.13.0 /usr/local/zig && \
    rm /tmp/zig.tar.xz
ENV PATH="/usr/local/zig:${PATH}"

# Install x86_64-elf cross-compiler (for linker compatibility)
RUN apt-get update && \
    apt-get install -y \
        gcc \
        binutils \
        && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

VOLUME /root/env
WORKDIR /root/env
```

- [ ] **Step 2: Build the image**

Run: `podman build -t carl_os_zig build_env/`
Expected: Successfully tagged localhost/carl_os_zig:latest

- [ ] **Step 3: Verify Zig is available**

Run: `podman run --rm carl_os_zig zig version`
Expected: `0.13.0`

- [ ] **Step 4: Commit**

```bash
git add build_env/Dockerfile
git commit -m "build: update Dockerfile with Zig and clean up packages"
```

---

### Task 3: Create build.zig with the build step

**Files:**
- Create: `build.zig`

**Interfaces:**
- Consumes: `build.zig.zon` (package manifest)
- Produces: `zig build` command that compiles kernel ISO inside Podman

- [ ] **Step 1: Create build.zig with container build step**

```zig
const std = @import("std");

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    // Container image build step
    const build_image = b.addSystemCommand(&.{
        "podman", "build", "-t", "carl_os_zig", "build_env/",
    });

    // Main build step - runs inside container
    const kernel_build = b.addSystemCommand(&.{
        "podman", "run", "--rm",
        "-v", "/Users/carlos/Developer/Projects/CarlOS:/root/env",
        "carl_os_zig",
        "make", "build-x86_64",
    });
    kernel_build.step.dependOn(&build_image.step);

    const build_step = b.step("build", "Build the kernel ISO");
    build_step.dependOn(&kernel_build.step);
}
```

- [ ] **Step 2: Test that build.zig parses**

Run: `zig build --help`
Expected: Shows "build" step in help output

- [ ] **Step 3: Commit**

```bash
git add build.zig
git commit -m "build: add initial build.zig with container build step"
```

---

### Task 4: Replace Makefile commands with Zig build commands inside container

**Files:**
- Modify: `build.zig`
- Modify: `build_env/Dockerfile` (add zig to PATH)

**Interfaces:**
- Consumes: Source files in `src/`, linker script in `targets/`
- Produces: `dist/x86_64/kernel.iso`

- [ ] **Step 1: Rewrite build.zig with direct compilation commands**

```zig
const std = @import("std");

pub fn build(b: *std.Build) void {
    // Container image build step
    const build_image = b.addSystemCommand(&.{
        "podman", "build", "-t", "carl_os_zig", "build_env/",
    });

    // The full build command to run inside the container
    const build_cmd =
        \\set -e && \
        \\mkdir -p build/x86_64/boot build/x86_64/disk build/kernel/main build/dist/x86_64 && \
        \\for f in src/main/x86_64/boot/*.asm; do \
        \\  nasm -f elf64 "$$f" -o "build/x86_64/boot/$$(basename "$$f" .asm).o"; \
        \\done && \
        \\for f in src/main/x86_64/*.asm; do \
        \\  nasm -f elf64 "$$f" -o "build/x86_64/$$(basename "$$f" .asm).o"; \
        \\done && \
        \\for f in src/main/x86_64/*.c; do \
        \\  zig cc -target x86_64-elf -ffreestanding -I src/main/interface -c "$$f" -o "build/x86_64/$$(basename "$$f" .c).o"; \
        \\done && \
        \\for f in src/main/kernel/*.c; do \
        \\  zig cc -target x86_64-elf -ffreestanding -I src/main/interface -c "$$f" -o "build/kernel/main/$$(basename "$$f" .c).o"; \
        \\done && \
        \\x86_64-elf-ld -n -o dist/x86_64/kernel.bin -T targets/x86_64/linker.ld \
        \\  build/kernel/main/*.o build/x86_64/*.o && \
        \\cp dist/x86_64/kernel.bin targets/x86_64/iso/boot/kernel.bin && \
        \\grub-mkrescue /usr/lib/grub/i386-pc -o dist/x86_64/kernel.iso targets/x86_64/iso
    ;

    // Main build step - runs inside container
    const kernel_build = b.addSystemCommand(&.{
        "podman", "run", "--rm",
        "-v", "/Users/carlos/Developer/Projects/CarlOS:/root/env",
        "carl_os_zig",
        "/bin/bash", "-c", build_cmd,
    });
    kernel_build.step.dependOn(&build_image.step);

    const build_step = b.step("build", "Build the kernel ISO");
    build_step.dependOn(&kernel_build.step);
}
```

- [ ] **Step 2: Test the build**

Run: `zig build`
Expected: `dist/x86_64/kernel.iso` is created

- [ ] **Step 3: Verify ISO exists**

Run: `ls -la dist/x86_64/kernel.iso`
Expected: File exists with non-zero size

- [ ] **Step 4: Commit**

```bash
git add build.zig
git commit -m "build: implement full kernel build in build.zig"
```

---

### Task 5: Add start step (build + run in QEMU)

**Files:**
- Modify: `build.zig`

**Interfaces:**
- Consumes: `dist/x86_64/kernel.iso` from build step
- Produces: Running QEMU instance

- [ ] **Step 1: Add start step to build.zig**

Add after the build step definition:

```zig
    // Create disk.img if it doesn't exist
    const create_disk = b.addSystemCommand(&.{
        "/bin/bash", "-c",
        \\if [ ! -f disk.img ]; then dd if=/dev/zero of=disk.img bs=1M count=10; fi
    });

    // Run in QEMU
    const run_qemu = b.addSystemCommand(&.{
        "qemu-system-x86_64",
        "-cdrom", "dist/x86_64/kernel.iso",
        "-drive", "file=disk.img,format=raw,index=0,media=disk",
    });
    run_qemu.step.dependOn(&kernel_build.step);
    run_qemu.step.dependOn(&create_disk.step);

    const start_step = b.step("start", "Build and run in QEMU");
    start_step.dependOn(&run_qemu.step);
```

- [ ] **Step 2: Test the start step**

Run: `zig build start`
Expected: QEMU launches with CarlOS (Ctrl+C to exit)

- [ ] **Step 3: Commit**

```bash
git add build.zig
git commit -m "build: add start step for QEMU execution"
```

---

### Task 6: Add clean step

**Files:**
- Modify: `build.zig`

**Interfaces:**
- Produces: Removes build artifacts and Podman resources

- [ ] **Step 1: Add clean step to build.zig**

Add after the start step definition:

```zig
    // Clean build artifacts
    const clean_build = b.addSystemCommand(&.{
        "rm", "-rf", "build", "dist",
    });

    // Clean Podman containers and images
    const clean_podman = b.addSystemCommand(&.{
        "/bin/bash", "-c",
        \\podman rm -f carl-os-build 2>/dev/null || true; \
        \\podman rmi carl_os_zig 2>/dev/null || true
    });

    const clean_step = b.step("clean", "Remove build artifacts and Podman resources");
    clean_step.dependOn(&clean_build.step);
    clean_step.dependOn(&clean_podman.step);
```

- [ ] **Step 2: Test the clean step**

Run: `zig build clean`
Expected: `build/` and `dist/` directories removed, Podman image removed

- [ ] **Step 3: Commit**

```bash
git add build.zig
git commit -m "build: add clean step for build artifacts and Podman cleanup"
```

---

### Task 7: Remove old Makefile and run.sh

**Files:**
- Delete: `Makefile`
- Delete: `run.sh`

**Interfaces:**
- N/A (cleanup)

- [ ] **Step 1: Delete old files**

```bash
git rm Makefile run.sh
```

- [ ] **Step 2: Update .gitignore**

Add to `.gitignore`:

```gitignore
# Zig build artifacts
build/
.zig-cache/
zig-out/
```

- [ ] **Step 3: Commit**

```bash
git add .gitignore
git commit -m "build: remove Makefile and run.sh, update .gitignore"
```

---

### Task 8: Final verification

**Files:**
- N/A (verification only)

**Interfaces:**
- N/A (end-to-end test)

- [ ] **Step 1: Clean everything**

Run: `zig build clean`
Expected: No errors, clean state

- [ ] **Step 2: Build from scratch**

Run: `zig build`
Expected: ISO created successfully

- [ ] **Step 3: Start in QEMU**

Run: `zig build start`
Expected: QEMU launches, CarlOS boots

- [ ] **Step 4: Final commit**

```bash
git add -A
git commit -m "build: complete migration to build.zig"
```
