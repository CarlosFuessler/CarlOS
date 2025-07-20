x86_64_c_src_files := $(shell find src/main/x86_64 -name *.c)
x86_64_c_obj_files := $(patsubst src/main/x86_64/%.c, build/x86_64/%.o, $(x86_64_c_src_files))

kernel_src_files := $(shell find src/main/kernel -name *.c)
kernel_obj_files := $(patsubst src/main/kernel/%.c, build/kernel/%.o, $(kernel_src_files))

x86_64_asm_src_files := $(shell find src/main/x86_64 -name *.asm)
x86_64_asm_obj_files := $(patsubst src/main/x86_64/%.asm, build/x86_64/%.o, $(x86_64_asm_src_files))

$(kernel_obj_files): build/kernel/%.o : src/main/kernel/%.c
	mkdir -p $(dir $@)
	x86_64-elf-gcc -c -I src/main/interface -ffreestanding $(patsubst build/kernel/%.o,src/main/kernel/%.c,$@) -o $@

$(x86_64_c_obj_files): build/x86_64/%.o : src/main/x86_64/%.c
	mkdir -p $(dir $@)
	x86_64-elf-gcc -c -I src/main/interface -ffreestanding $(patsubst build/x86_64/%.o,src/main/x86_64/%.c,$@) -o $@

$(x86_64_asm_obj_files): build/x86_64/%.o : src/main/x86_64/%.asm
	mkdir -p $(dir $@)
	nasm -f elf64 $< -o $@

.PHONY: build-x86_64

build-x86_64: $(kernel_obj_files) $(x86_64_c_obj_files) $(x86_64_asm_obj_files)
	mkdir -p dist/x86_64
	x86_64-elf-ld -n -o dist/x86_64/kernel.bin -T targets/x86_64/linker.ld $(kernel_obj_files) $(x86_64_c_obj_files) $(x86_64_asm_obj_files)
	cp dist/x86_64/kernel.bin targets/x86_64/iso/boot/kernel.bin
	grub-mkrescue /usr/lib/grub/i386-pc -o dist/x86_64/kernel.iso targets/x86_64/iso