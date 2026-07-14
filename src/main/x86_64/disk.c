#include "disk.h"

// read a byte from an I/O port
static inline uint8_t inb(uint16_t port)
{
    uint8_t result;
    __asm__ volatile("inb %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

// write a byte to an I/O port
static inline void outb(uint16_t port, uint8_t value)
{
    __asm__ volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

// read multiple 32-bit values from an I/O port into memory
static inline void insl(uint16_t port, void *addr, uint32_t count)
{
    __asm__ volatile("cld; rep insl" : "+D"(addr), "+c"(count) : "d"(port) : "memory");
}

// rrite multiple 32-bit values from memory to an I/O port
static inline void outsl(uint16_t port, const void *addr, uint32_t count)
{
    __asm__ volatile("cld; rep outsl" : "+S"(addr), "+c"(count) : "d"(port) : "memory");
}

static void disk_wait_busy(void)
{
    while (inb(ATA_PRIMARY_IO + 7) & ATA_SR_BSY)
        ;
}

// ait until disk is ready
static void disk_wait_ready(void)
{
    while (!(inb(ATA_PRIMARY_IO + 7) & ATA_SR_DRDY))
        ;
}

// wait until disk has data ready to read
static void disk_wait_drq(void)
{
    while (!(inb(ATA_PRIMARY_IO + 7) & ATA_SR_DRQ))
        ;
}

// nitialize the disk controller
void disk_init(void)
{
    disk_wait_busy();
    disk_wait_ready();
}

// read one sector (512 bytes) from disk at logical block address
// returns 0 on success, -1 on error
int disk_read_sector(uint32_t lba, uint8_t *buffer)
{
    disk_wait_busy();

    // set LBA address and sector count
    outb(ATA_PRIMARY_IO + 6, 0xE0 | ((lba >> 24) & 0x0F));
    outb(ATA_PRIMARY_IO + 2, 1);
    outb(ATA_PRIMARY_IO + 3, (uint8_t)lba);
    outb(ATA_PRIMARY_IO + 4, (uint8_t)(lba >> 8));
    outb(ATA_PRIMARY_IO + 5, (uint8_t)(lba >> 16));
    outb(ATA_PRIMARY_IO + 7, ATA_CMD_READ_SECTORS);

    // it for data and read 512 bytes into buffer
    disk_wait_drq();
    insl(ATA_PRIMARY_IO, buffer, SECTOR_SIZE / 4);

    // Check for errors
    if (inb(ATA_PRIMARY_IO + 7) & ATA_SR_ERR)
    {
        return -1;
    }

    return 0;
}

// rite one sector (512 bytes) to disk at logical block address
// eturns 0 on success, -1 on error
int disk_write_sector(uint32_t lba, const uint8_t *buffer)
{
    disk_wait_busy();

    // set LBA address and sector count
    outb(ATA_PRIMARY_IO + 6, 0xE0 | ((lba >> 24) & 0x0F));
    outb(ATA_PRIMARY_IO + 2, 1);
    outb(ATA_PRIMARY_IO + 3, (uint8_t)lba);
    outb(ATA_PRIMARY_IO + 4, (uint8_t)(lba >> 8));
    outb(ATA_PRIMARY_IO + 5, (uint8_t)(lba >> 16));
    outb(ATA_PRIMARY_IO + 7, ATA_CMD_WRITE_SECTORS);

    // wait for ready and write 512 bytes from buffer
    disk_wait_drq();
    outsl(ATA_PRIMARY_IO, buffer, SECTOR_SIZE / 4);

    // flush cache
    outb(ATA_PRIMARY_IO + 7, 0xE7);
    disk_wait_busy();

    // check for errors
    if (inb(ATA_PRIMARY_IO + 7) & ATA_SR_ERR)
    {
        return -1;
    }

    return 0;
}