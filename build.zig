const std = @import("std");

pub fn build(b: *std.Build) void {
    const root_path = b.build_root.path orelse ".";
    const mount_arg = std.fmt.allocPrint(b.allocator, "{s}:/root/env", .{root_path}) catch unreachable;

    // Step 1: Compile ASM files with nasm
    const asm_cmd =
        \\set -e && \
        \\mkdir -p build/x86_64/boot && \
        \\for f in $(find src/main/x86_64 -name '*.asm'); do \
        \\  dir="build/x86_64/$(dirname "${f#src/main/x86_64/}")"; \
        \\  mkdir -p "$dir"; \
        \\  echo "nasm $f"; \
        \\  nasm -f elf64 "$f" -o "$dir/$(basename "$f" .asm).o"; \
        \\done
    ;
    const asm_build = b.addSystemCommand(&.{ "/bin/bash", "-c", asm_cmd });

    // Step 2: Compile C files with zig cc (freestanding, no UBSan)
    const c_cmd =
        \\set -e && \
        \\mkdir -p build/x86_64/boot build/kernel/main && \
        \\for f in $(find src/main/x86_64 -name '*.c'); do \
        \\  dir="build/x86_64/$(dirname "${f#src/main/x86_64/}")"; \
        \\  mkdir -p "$dir"; \
        \\  echo "cc $f"; \
        \\  zig cc -target x86_64-freestanding -ffreestanding -fno-sanitize=all -I src/main/interface -c "$f" -o "$dir/$(basename "$f" .c).o"; \
        \\done && \
        \\for f in $(find src/main/kernel -name '*.c'); do \
        \\  dir="build/kernel/$(dirname "${f#src/main/kernel/}")"; \
        \\  mkdir -p "$dir"; \
        \\  echo "cc $f"; \
        \\  zig cc -target x86_64-freestanding -ffreestanding -fno-sanitize=all -I src/main/interface -c "$f" -o "$dir/$(basename "$f" .c).o"; \
        \\done
    ;
    const c_build = b.addSystemCommand(&.{ "/bin/bash", "-c", c_cmd });
    c_build.step.dependOn(&asm_build.step);

    // Step 3: Link with ld.lld
    const link_cmd =
        \\set -e
        \\mkdir -p dist/x86_64
        \\objects=$(find build -name '*.o' -type f)
        \\ld.lld -n -o dist/x86_64/kernel.bin -T targets/x86_64/linker.ld $objects
        \\echo "Link done"
    ;
    const link_build = b.addSystemCommand(&.{ "/bin/bash", "-c", link_cmd });
    link_build.step.dependOn(&c_build.step);

    // Step 4: Create ISO with grub-mkrescue via Podman
    const iso_cmd =
        \\set -e && \
        \\cp dist/x86_64/kernel.bin targets/x86_64/iso/boot/kernel.bin && \
        \\grub-mkrescue /usr/lib/grub/i386-pc -o dist/x86_64/kernel.iso targets/x86_64/iso && \
        \\echo "ISO created"
    ;
    const iso_build = b.addSystemCommand(&.{
        "podman", "run", "--rm",
        "-v", mount_arg,
        "carl_os_zig",
        "/bin/bash", "-c", iso_cmd,
    });
    iso_build.step.dependOn(&link_build.step);

    const build_step = b.step("build", "Build the kernel ISO");
    build_step.dependOn(&iso_build.step);

    // Run in QEMU
    const create_disk = b.addSystemCommand(&.{
        "/bin/bash", "-c",
        \\if [ ! -f disk.img ]; then dd if=/dev/zero of=disk.img bs=1M count=10; fi
    });

    const run_qemu = b.addSystemCommand(&.{
        "qemu-system-x86_64",
        "-cdrom", "dist/x86_64/kernel.iso",
        "-drive", "file=disk.img,format=raw,index=0,media=disk",
    });
    run_qemu.step.dependOn(&iso_build.step);
    run_qemu.step.dependOn(&create_disk.step);

    const start_step = b.step("start", "Build and run in QEMU");
    start_step.dependOn(&run_qemu.step);

    // Clean
    const clean_build_cmd = b.addSystemCommand(&.{ "rm", "-rf", "build", "dist" });
    const clean_podman = b.addSystemCommand(&.{
        "/bin/bash", "-c",
        \\podman rm -f carl-os-build 2>/dev/null || true; \
        \\podman rmi carl_os_zig 2>/dev/null || true
    });
    const clean_step = b.step("clean", "Remove build artifacts and Podman resources");
    clean_step.dependOn(&clean_build_cmd.step);
    clean_step.dependOn(&clean_podman.step);
}
