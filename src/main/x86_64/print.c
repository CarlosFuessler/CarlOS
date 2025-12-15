#include "print.h"
#include <stddef.h> // for size_t
#include <string.h>

static const size_t NUM_COLS = 80;
static const size_t NUM_ROWS = 25;

struct Char
{
    unsigned char character;
    unsigned char color;
};

struct Char *buffer = (struct Char *)0xb8000;
size_t col = 0;
size_t row = 0;
unsigned char color = 0x0f;

void print_clear(void)
{
    col = 0;
    row = 0;

    for (size_t i = 0; i < NUM_ROWS; i++)
    {
        for (size_t j = 0; j < NUM_COLS; j++)
        {
            buffer[i * NUM_COLS + j] = (struct Char){
                .character = ' ',
                .color = color,
            };
        }
    }
}

void print_char(char character)
{
    if (character == '\n')
    {
        col = 0;
        row++;
        return;
    }

    if (col >= NUM_COLS)
    {
        col = 0;
        row++;
    }

    if (row >= NUM_ROWS)
    {
        row = 0;
    }

    buffer[row * NUM_COLS + col] = (struct Char){
        .character = character,
        .color = color,
    };

    col++;
}

void print_str(char *string)
{
    for (size_t i = 0; string[i] != '\0'; i++)
    {
        print_char(string[i]);
    }
}

void print_set_color(unsigned char foreground, unsigned char background)
{
    color = foreground + (background << 4);
}

void print_set_cursor(size_t new_col, size_t new_row)
{
    if (new_col < NUM_COLS && new_row < NUM_ROWS)
    {
        col = new_col;
        row = new_row;
    }
}

size_t print_get_col(void)
{
    return col;
}

size_t print_get_row(void)
{
    return row;
}

void print_newline(void)
{
    col = 0;
    row++;
    if (row >= NUM_ROWS)
    {
        print_clear();
        row = 0;
    }
}

void delete_char(void)
{
    // Check if we are at the start of the line
    if (col > 0)
    {
        col--; // move one character back

        // Clear the character on screen
        buffer[row * NUM_COLS + col] = (struct Char){
            .character = ' ',
            .color = color,
        };
    }
    else if (row > 0)
    {
        // Go to the previous line (near the end)
        row--;
        col = NUM_COLS - 1;

        // Find the last character in the previous line
        while (col > 0 && buffer[row * NUM_COLS + col].character == ' ')
        {
            col--;
        }

        // Clear the found character
        buffer[row * NUM_COLS + col] = (struct Char){
            .character = ' ',
            .color = color,
        };
    }
}

void print_logo_goodbye(void)
{
    print_set_color(PRINT_COLOR_PINK, PRINT_COLOR_BLACK);
    print_str("           ___________                     \n");
    print_str("       __##################__                \n");
    print_set_color(PRINT_COLOR_MAGENTA, PRINT_COLOR_BLACK);
    print_str("     |#######################|               \n");
    print_str("   |############_____                          \n");
    print_set_color(PRINT_COLOR_LIGHT_BLUE, PRINT_COLOR_BLACK);
    print_str("  |##########                             \n");
    print_str(" |#########        .=========.           \n");
    print_set_color(PRINT_COLOR_BLUE, PRINT_COLOR_BLACK);
    print_str("########          |#########|          \n");
    print_str(" |########         |#########|        \n");
    print_set_color(PRINT_COLOR_YELLOW, PRINT_COLOR_BLACK);
    print_str("  |########        |#########|        \n");
    print_str("   |#########################| oodbye  \n");
    print_set_color(PRINT_COLOR_GREEN, PRINT_COLOR_BLACK);
    print_str("     |######################|           \n");
    print_str("       --##################--               \n");
}

void print_logo(void)
{
    print_set_color(PRINT_COLOR_PINK, PRINT_COLOR_BLACK);
    print_str("           ___________               \n");
    print_str("       __##################__          \n");
    print_set_color(PRINT_COLOR_MAGENTA, PRINT_COLOR_BLACK);
    print_str("     |#######################|         \n");
    print_str("   |#############_____                \n");
    print_set_color(PRINT_COLOR_LIGHT_BLUE, PRINT_COLOR_BLACK);
    print_str("  |############                  \n");
    print_str(" |############                   \n");
    print_set_color(PRINT_COLOR_BLUE, PRINT_COLOR_BLACK);
    print_str("############                    \n");
    print_str(" |############                   \n");
    print_set_color(PRINT_COLOR_YELLOW, PRINT_COLOR_BLACK);
    print_str("  |############                  \n");
    print_str("   |#############_____                \n");
    print_set_color(PRINT_COLOR_GREEN, PRINT_COLOR_BLACK);
    print_str("     |#################| arl0S        \n");
    print_str("       --##################--         \n");
    print_set_color(PRINT_COLOR_LIGHT_GREEN, PRINT_COLOR_BLACK);
    print_str("        --|#############|--          \n");
    print_str("           -----------              \n");
}

void print_logo_welcome(void)
{
    print_set_color(PRINT_COLOR_PINK, PRINT_COLOR_BLACK);
    print_str(" .======.                         .======.  \n");
    print_str(" | #### |                         | #### | \n");
    print_set_color(PRINT_COLOR_BLUE, PRINT_COLOR_BLACK);
    print_str("  | ### |                        | ### |          \n");
    print_str("   | ## |        .===.           | ## |        \n");
    print_set_color(PRINT_COLOR_LIGHT_BLUE, PRINT_COLOR_BLACK);
    print_str("    | # |      .=====.           | # |        \n");
    print_str("     | #====================###### | elcome \n");
    print_set_color(PRINT_COLOR_WHITE, PRINT_COLOR_BLACK);
    print_str("       | ########        ######## |          \n");
    print_str("        `======='       '========'              \n\n\n");
}
