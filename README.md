# CarlOS

A simple hobby OS written in ASM and C.

## Build / Run

The easiest way is to build inside Docker and run with QEMU on your host.

```bash
# 1) Build the Docker image (contains the cross toolchain)
docker build -t carl_os build_env

# 2) Build in Docker and boot in QEMU (also creates disk.img if missing)
./run.sh
```

### Notes

- Host requirements: Docker + `qemu-system-x86_64`.
- The script writes the build output to `dist/` and uses `disk.img` as a raw disk.

### Manual build inside the container (optional)

```bash
docker run --rm -it -v "$(pwd)":/root/env carl_os
make build-x86_64
exit

# then on the host
qemu-system-x86_64 \
	-cdrom dist/x86_64/kernel.iso \
	-drive file=disk.img,format=raw,index=0,media=disk
```

## Project structure

- `src/main/x86_64/boot/` - boot assembly
- `src/main/kernel/` - kernel C code
- `src/main/interface/` - headers
- `targets/x86_64/` - linker script + ISO/GRUB config

## Features

- 32-bit to 64-bit long mode switch
- VGA text output
- Basic printing
- String helpers (int/hex conversion)
- Simple shell
- Calculator
- Basic disk + simple file system

## Not implemented yet

- advanced memory management
- 3D graphics
![plot](Assets/about.png)

