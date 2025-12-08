#include "keyboard.h"
#include "print.h"

// Keyboard Controller Ports
#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_STATUS_PORT 0x64
#define KEYBOARD_COMMAND_PORT 0x64

// Status Bits
#define KEYBOARD_STATUS_OUTPUT_FULL 0x01
#define KEYBOARD_STATUS_INPUT_FULL 0x02

// Globale Variablen
static keyboard_status_t keyboard_status = KEYBOARD_NOT_INITIALIZED;
static char input_buffer[256];
static size_t buffer_read_pos = 0;
static size_t buffer_write_pos = 0;
static size_t buffer_count = 0;

// Deutsche Tastatur Layout
static const char scancode_to_ascii_table[128] = {
    0, KEY_ESCAPE, '1', '2', '3', '4', '5', '6',           // 0x00-0x07
    '7', '8', '9', '0', 's', '\'', KEY_BACKSPACE, KEY_TAB, // 0x08-0x0F
    'q', 'w', 'e', 'r', 't', 'z', 'u', 'i',                // 0x10-0x17
    'o', 'p', 'u', '+', KEY_ENTER, 0, 'a', 's',            // 0x18-0x1F
    'd', 'f', 'g', 'h', 'j', 'k', 'l', 'o',                // 0x20-0x27
    'a', '$', 0, '#', 'y', 'x', 'c', 'v',                  // 0x28-0x2F
    'b', 'n', 'm', '=', '.', '-', 0, '*',                  // 0x30-0x37
    0, KEY_SPACE, 0, 0, 0, 0, 0, 0,                        // 0x38-0x3F
    0, 0, 0, 0, 0, 0, 0, 0,                                // 0x40-0x47
    0, 0, 0, 0, 0, 0, 0, 0,                                // 0x48-0x4F
    0, 0, 0, 0, 0, 0, 0, 0,                                // 0x50-0x57
    0, 0, 0, 0, 0, 0, 0, 0,                                // 0x58-0x5F
    0, 0, 0, 0, 0, 0, 0, 0,                                // 0x60-0x67
    0, 0, 0, 0, 0, 0, 0, 0,                                // 0x68-0x6F
    0, 0, 0, 0, 0, 0, 0, 0,                                // 0x70-0x77
    0, 0, 0, 0, 0, 0, 0, 0                                 // 0x78-0x7F
};

// Port I/O
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

// Warte bis Controller bereit
static void keyboard_wait_controller_ready(void)
{
    while (inb(KEYBOARD_STATUS_PORT) & KEYBOARD_STATUS_INPUT_FULL)
    {
    }
}

static void keyboard_wait_for_data(void)
{
    while (!(inb(KEYBOARD_STATUS_PORT) & KEYBOARD_STATUS_OUTPUT_FULL))
    {
    }
}

// Buffer Management
static void buffer_add_char(char c)
{
    if (buffer_count < sizeof(input_buffer))
    {
        input_buffer[buffer_write_pos] = c;
        buffer_write_pos = (buffer_write_pos + 1) % sizeof(input_buffer);
        buffer_count++;
    }
}

static char buffer_get_char(void)
{
    if (buffer_count == 0)
    {
        return 0;
    }

    char c = input_buffer[buffer_read_pos];
    buffer_read_pos = (buffer_read_pos + 1) % sizeof(input_buffer);
    buffer_count--;
    return c;
}

void keyboard_init(void)
{
    // Reset Buffer
    buffer_read_pos = 0;
    buffer_write_pos = 0;
    buffer_count = 0;

    while (inb(KEYBOARD_STATUS_PORT) & KEYBOARD_STATUS_OUTPUT_FULL)
    {
        inb(KEYBOARD_DATA_PORT);
    }

    keyboard_status = KEYBOARD_READY;
}

uint8_t keyboard_read_scancode(void)
{
    if (!(inb(KEYBOARD_STATUS_PORT) & KEYBOARD_STATUS_OUTPUT_FULL))
    {
        return 0;
    }

    return inb(KEYBOARD_DATA_PORT);
}

char scancode_to_ascii(uint8_t scancode)
{
    if (scancode & 0x80)
    {
        return 0;
    }

    if (scancode >= sizeof(scancode_to_ascii_table))
    {
        return 0;
    }

    return scancode_to_ascii_table[scancode];
}

void keyboard_process_input(void)
{
    uint8_t scancode = keyboard_read_scancode();

    if (scancode != 0)
    {
        char ascii = scancode_to_ascii(scancode);
        if (ascii != 0)
        {
            buffer_add_char(ascii);
        }
    }
}

int keyboard_has_input(void)
{
    keyboard_process_input();
    return buffer_count > 0;
}

char keyboard_get_char(void)
{
    while (!keyboard_has_input())
    {
    }

    return buffer_get_char();
}

void keyboard_wait_for_key(void)
{
    keyboard_get_char();
}

keyboard_status_t keyboard_get_status(void)
{
    return keyboard_status;
}