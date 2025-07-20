# =============================================================================
# Variablen
# =============================================================================
SRC_DIR = src
BUILD_DIR = build
TOOLS_DIR = tools

ASM = nasm
CC = gcc

# =============================================================================
# Ziele
# =============================================================================
.PHONY: all floppy_image kernel bootloader tools clean

# Standardziel, das beim Aufruf von 'make' ausgeführt wird
all: floppy_image

# Hauptziel: Erstellt das Disketten-Image
floppy_image: $(BUILD_DIR)/main_floppy.img

# Ziel zum Assemblieren des Bootloaders
bootloader: $(BUILD_DIR)/bootloader.bin

# Ziel zum Assemblieren des Kernels
kernel: $(BUILD_DIR)/kernel.bin

# Ziel zum Aufräumen des Build-Verzeichnisses
clean:
	@echo "Cleaning up build directory..."
	@rm -rf $(BUILD_DIR)

# =============================================================================
# Build-Regeln
# =============================================================================

# Regel zum Erstellen des finalen Disketten-Images
$(BUILD_DIR)/main_floppy.img: $(BUILD_DIR)/bootloader.bin $(BUILD_DIR)/kernel.bin
	@echo "Creating floppy image..."
	@mkdir -p $(BUILD_DIR)
	@dd if=/dev/zero of=$@ bs=512 count=2880 > /dev/null 2>&1
	@mkfs.fat -F 12 -n "CarlOS" $@ > /dev/null 2>&1
    
	@echo "Injecting bootloader code into the boot sector..."
	@dd if=$(BUILD_DIR)/bootloader.bin of=$@ conv=notrunc > /dev/null 2>&1
    
	@echo "Copying kernel to floppy image..."
	@mcopy -i $@ $(BUILD_DIR)/kernel.bin "::KERNEL.BIN" > /dev/null 2>&1
    
	@echo "Image created successfully"
# Regel zum Assemblieren des Bootloaders
$(BUILD_DIR)/bootloader.bin: $(SRC_DIR)/bootloader/boot.asm
	@echo "Assembling bootloader: $@"
	@mkdir -p $(BUILD_DIR)
	@$(ASM) $< -f bin -o $@

# Regel zum Assemblieren des Kernels
$(BUILD_DIR)/kernel.bin: $(SRC_DIR)/kernel/main.asm
	@echo "Assembling kernel: $@"
	@mkdir -p $(BUILD_DIR)
	@$(ASM) $< -f bin -o $@