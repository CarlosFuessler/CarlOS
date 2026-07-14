const std = @import("std");

pub fn build(b: *std.Build) void {
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
