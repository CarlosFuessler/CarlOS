#include "disk.h"

static inline uint8_t inb(uint16_t port)
{
    uint8_t result;
    __asm__ volatile("inb %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

static inline void outb(uint16_t port, uint8_t value)
{
    __asm__ volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline void insl(uint16_t port, void *addr, uint32_t count)
{
    __asm__ volatile("cld; rep insl" : "+D"(addr), "+c"(count) : "d"(port) : "memory");
}

static inline void outsl(uint16_t port, const void *addr, uint32_t count)
{
    __asm__ volatile("cld; rep outsl" : "+S"(addr), "+c"(count) : "d"(port) : "memory");
}

static void disk_wait_busy(void)
{
    while (inb(ATA_PRIMARY_IO + 7) & ATA_SR_BSY)
        ;
}

static void disk_wait_ready(void)
{
    while (!(inb(ATA_PRIMARY_IO + 7) & ATA_SR_DRDY))
        ;
}

static void disk_wait_drq(void)
{
    while (!(inb(ATA_PRIMARY_IO + 7) & ATA_SR_DRQ))
        ;
}

void disk_init(void)
{
    disk_wait_busy();
    disk_wait_ready();
}

int disk_read_sector(uint32_t lba, uint8_t *buffer)
{
    disk_wait_busy();

    outb(ATA_PRIMARY_IO + 6, 0xE0 | ((lba >> 24) & 0x0F));
    outb(ATA_PRIMARY_IO + 2, 1);
    outb(ATA_PRIMARY_IO + 3, (uint8_t)lba);
    outb(ATA_PRIMARY_IO + 4, (uint8_t)(lba >> 8));
    outb(ATA_PRIMARY_IO + 5, (uint8_t)(lba >> 16));
    outb(ATA_PRIMARY_IO + 7, ATA_CMD_READ_SECTORS);

    disk_wait_drq();
    insl(ATA_PRIMARY_IO, buffer, SECTOR_SIZE / 4);

    if (inb(ATA_PRIMARY_IO + 7) & ATA_SR_ERR)
    {
        return -1;
    }

    return 0;
}

int disk_write_sector(uint32_t lba, const uint8_t *buffer)
{
    disk_wait_busy();

    outb(ATA_PRIMARY_IO + 6, 0xE0 | ((lba >> 24) & 0x0F));
    outb(ATA_PRIMARY_IO + 2, 1);
    outb(ATA_PRIMARY_IO + 3, (uint8_t)lba);
    outb(ATA_PRIMARY_IO + 4, (uint8_t)(lba >> 8));
    outb(ATA_PRIMARY_IO + 5, (uint8_t)(lba >> 16));
    outb(ATA_PRIMARY_IO + 7, ATA_CMD_WRITE_SECTORS);

    disk_wait_drq();
    outsl(ATA_PRIMARY_IO, buffer, SECTOR_SIZE / 4);

    outb(ATA_PRIMARY_IO + 7, 0xE7);
    disk_wait_busy();

    if (inb(ATA_PRIMARY_IO + 7) & ATA_SR_ERR)
    {
        return -1;
    }

    return 0;
}
