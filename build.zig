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
}
