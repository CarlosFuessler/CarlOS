#include "keyboard.h"
#include "print.h"

// ========== KONSTANTEN ==========
// PS/2 Keyboard Controller Ports
#define KEYBOARD_DATA_PORT 0x60    // Daten-Port
#define KEYBOARD_STATUS_PORT 0x64  // Status-Port
#define KEYBOARD_COMMAND_PORT 0x64 // Kommando-Port

// Status-Register Bits
#define KEYBOARD_STATUS_OUTPUT_FULL 0x01 // Ausgabe-Buffer ist voll
#define KEYBOARD_STATUS_INPUT_FULL 0x02  // Eingabe-Buffer ist voll

// ========== GLOBALE VARIABLEN ==========
static keyboard_status_t keyboard_status = KEYBOARD_NOT_INITIALIZED;
static char input_buffer[256];      // Ring-Buffer für Eingaben
static size_t buffer_read_pos = 0;  // Lese-Position im Buffer
static size_t buffer_write_pos = 0; // Schreib-Position im Buffer
static size_t buffer_count = 0;     // Anzahl Zeichen im Buffer

// Deutsche Tastatur mit korrekter < Taste (links neben Y/Z)
static const char scancode_to_ascii_table[128] = {
    0, KEY_ESCAPE, '1', '2', '3', '4', '5', '6',           // 0x00-0x07
    '7', '8', '9', '0', 's', '\'', KEY_BACKSPACE, KEY_TAB, // 0x08-0x0F
    'q', 'w', 'e', 'r', 't', 'z', 'u', 'i',                // 0x10-0x17
    'o', 'p', 'u', '+', KEY_ENTER, 0, 'a', 's',            // 0x18-0x1F
    'd', 'f', 'g', 'h', 'j', 'k', 'l', 'o',                // 0x20-0x27
    'a', '$', 0, '#', 'y', 'x', 'c', 'v',                  // 0x28-0x2F
    'b', 'n', 'm', ',', '.', '-', 0, '*',                  // 0x30-0x37
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
// ========== LOW-LEVEL I/O FUNKTIONEN ==========
// Liest ein Byte von einem Port
static inline uint8_t inb(uint16_t port)
{
    uint8_t result;
    __asm__ volatile("inb %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

// Schreibt ein Byte zu einem Port
static inline void outb(uint16_t port, uint8_t value)
{
    __asm__ volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

// ========== KEYBOARD CONTROLLER FUNKTIONEN ==========
// Wartet bis der Keyboard Controller bereit ist
static void keyboard_wait_controller_ready(void)
{
    while (inb(KEYBOARD_STATUS_PORT) & KEYBOARD_STATUS_INPUT_FULL)
    {
        // Warte bis Input-Buffer leer ist
    }
}

// Wartet auf Daten vom Keyboard
static void keyboard_wait_for_data(void)
{
    while (!(inb(KEYBOARD_STATUS_PORT) & KEYBOARD_STATUS_OUTPUT_FULL))
    {
        // Warte bis Output-Buffer voll ist
    }
}

// ========== BUFFER-MANAGEMENT ==========
// Fügt ein Zeichen zum Ring-Buffer hinzu
static void buffer_add_char(char c)
{
    if (buffer_count < sizeof(input_buffer))
    {
        input_buffer[buffer_write_pos] = c;
        buffer_write_pos = (buffer_write_pos + 1) % sizeof(input_buffer);
        buffer_count++;
    }
}

// Liest ein Zeichen aus dem Ring-Buffer
static char buffer_get_char(void)
{
    if (buffer_count == 0)
    {
        return 0; // Buffer ist leer
    }

    char c = input_buffer[buffer_read_pos];
    buffer_read_pos = (buffer_read_pos + 1) % sizeof(input_buffer);
    buffer_count--;
    return c;
}

// ========== ÖFFENTLICHE FUNKTIONEN ==========
// Initialisiert das Keyboard-System
void keyboard_init(void)
{
    // print_str("Keyboard...\n");

    // Reset Buffer-Variablen
    buffer_read_pos = 0;
    buffer_write_pos = 0;
    buffer_count = 0;

    // Leere den Keyboard-Buffer
    while (inb(KEYBOARD_STATUS_PORT) & KEYBOARD_STATUS_OUTPUT_FULL)
    {
        inb(KEYBOARD_DATA_PORT);
    }

    keyboard_status = KEYBOARD_READY;
    // print_str("Keyboard: ready!\n");
}

// Liest einen Scancode direkt vom Keyboard
uint8_t keyboard_read_scancode(void)
{
    // Prüfe ob Daten verfügbar sind
    if (!(inb(KEYBOARD_STATUS_PORT) & KEYBOARD_STATUS_OUTPUT_FULL))
    {
        return 0; // Keine Daten verfügbar
    }

    return inb(KEYBOARD_DATA_PORT);
}

// Konvertiert einen Scancode zu ASCII
char scancode_to_ascii(uint8_t scancode)
{
    // Ignoriere Release-Codes (Bit 7 gesetzt)
    if (scancode & 0x80)
    {
        return 0;
    }

    // Prüfe gültigen Bereich
    if (scancode >= sizeof(scancode_to_ascii_table))
    {
        return 0;
    }

    return scancode_to_ascii_table[scancode];
}

// Verarbeitet Keyboard-Input und füllt den Buffer
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

// Prüft ob Eingaben verfügbar sind
int keyboard_has_input(void)
{
    keyboard_process_input(); // Verarbeite neue Eingaben
    return buffer_count > 0;
}

// Holt das nächste Zeichen (blockierend wenn nötig)
char keyboard_get_char(void)
{
    // Warte bis ein Zeichen verfügbar ist
    while (!keyboard_has_input())
    {
        // Polling - in einem echten OS würde man hier Interrupts verwenden
    }

    return buffer_get_char();
}

// Wartet auf einen Tastendruck
void keyboard_wait_for_key(void)
{
    keyboard_get_char(); // Blockiert bis eine Taste gedrückt wird
}

// Gibt den Keyboard-Status zurück
keyboard_status_t keyboard_get_status(void)
{
    return keyboard_status;
}