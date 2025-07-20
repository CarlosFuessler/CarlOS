#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv)
{
    if (argc < 3)
    {
        printf("Syntax: %s <disk image> <bootloader>\n", argv[0]);
        return -1;
    }

    FILE *disk = fopen(argv[1], "rb+");
    if (!disk)
    {
        fprintf(stderr, "Cannot open disk image: %s\n", argv[1]);
        return -1;
    }

    FILE *bootloader = fopen(argv[2], "rb");
    if (!bootloader)
    {
        fprintf(stderr, "Cannot open bootloader file: %s\n", argv[2]);
        fclose(disk);
        return -1;
    }

    char bootloader_data[512];
    int read_count = fread(bootloader_data, 1, 512, bootloader);
    if (read_count != 512)
    {
        fprintf(stderr, "Bootloader is not 512 bytes!\n");
        fclose(bootloader);
        fclose(disk);
        return -1;
    }

    // Jump-Anweisung und OEM-ID aus dem Original-Bootsektor beibehalten
    char original_boot_sector[11];
    if (fseek(disk, 0, SEEK_SET) != 0 || fread(original_boot_sector, 1, 11, disk) != 11)
    {
        fprintf(stderr, "Could not read original boot sector from disk image.\n");
        fclose(bootloader);
        fclose(disk);
        return -1;
    }

    // Kopiere die ersten 11 Bytes (JMP + OEM) in unseren Bootloader-Puffer
    memcpy(bootloader_data, original_boot_sector, 11);

    // Schreibe den modifizierten Bootloader zurück auf die Disk
    if (fseek(disk, 0, SEEK_SET) != 0 || fwrite(bootloader_data, 1, 512, disk) != 512)
    {
        fprintf(stderr, "Could not write bootloader to disk image.\n");
        fclose(bootloader);
        fclose(disk);
        return -1;
    }

    printf("Bootloader successfully written to %s\n", argv[1]);
    fclose(bootloader);
    fclose(disk);
    return 0;
}