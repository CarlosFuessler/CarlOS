#pragma once

#include <stdint.h>
#include <stddef.h>

// Keyboard status
typedef enum
{
    KEYBOARD_NOT_INITIALIZED = 0,
    KEYBOARD_READY = 1
} keyboard_status_t;

// Special keys
#define KEY_BACKSPACE 0x08
#define KEY_TAB 0x09
#define KEY_ENTER 0x0A
#define KEY_ESCAPE 0x1B
#define KEY_SPACE 0x20

// Keyboard management functions
void keyboard_init(void);
char keyboard_get_char(void);
int keyboard_has_input(void);
void keyboard_wait_for_key(void);

#define KEY_LESS_THAN 0x3C

// Low-level functions
uint8_t keyboard_read_scancode(void);
char scancode_to_ascii(uint8_t scancode);

// Status functions
keyboard_status_t keyboard_get_status(void);