#pragma once

#include <stdint.h>
#include <stddef.h>

#define SECTOR_SIZE 512

// IDE Ports
#define ATA_PRIMARY_IO 0x1F0
#define ATA_PRIMARY_CONTROL 0x3F6

// ATA Befehle
#define ATA_CMD_READ_SECTORS 0x20
#define ATA_CMD_WRITE_SECTORS 0x30
#define ATA_CMD_IDENTIFY 0xEC

// Status Bits
#define ATA_SR_BSY 0x80
#define ATA_SR_DRDY 0x40
#define ATA_SR_DRQ 0x08
#define ATA_SR_ERR 0x01

void disk_init(void);
int disk_read_sector(uint32_t lba, uint8_t *buffer);
int disk_write_sector(uint32_t lba, const uint8_t *buffer);
