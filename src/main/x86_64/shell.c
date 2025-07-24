#include "shell.h"
#include "keyboard.h"
#include "print.h"
#include "string.h"

// GLOBALE VARIABLEN
static char input_buffer[SHELL_BUFFER_SIZE];
static size_t buffer_pos = 0;
uint8_t forground_color = PRINT_COLOR_PINK;
uint8_t background_color = PRINT_COLOR_BLACK;

// SHELL INITIALISIERUNG
void shell_init(void)
{
    print_clear();
    print_set_color(forground_color, background_color);
    print_str("\n===CarlOS Shell v1.0===\n\n");
    print_str("Wirte <help> for a list of all commands!\n\n");
}

// COMMAND PROCESSING
void shell_process_command(const char *command)
{
    // Entferne führende/nachfolgende Leerzeichen
    while (*command == ' ')
        command++;

    // Leerer Befehl - ignorieren
    if (strlen(command) == 0)
    {
        return;
    }

    // Command-Vergleich
    if (strcmp(command, "about") == 0)
    {
        cmd_about();
    }
    else if (strcmp(command, "help") == 0)
    {
        cmd_help();
    }
    else if (strcmp(command, "clear") == 0)
    {
        print_clear();
    }
    else if (strcmp(command, "color") == 0)
    {
        switch_color();
    }
    else if (strcmp(command, "echo") == 0)
    {
        echo();
    }
    else

    {
        print_clear();
        print_str("unknown command: ");
        print_str(command);
        print_str("\nwrite <help> for unknown commands \n");
    }
}

// SHELL HAUPTSCHLEIFE
void shell_run(void)
{
    buffer_pos = 0;

    // Zeige Prompt
    print_str("\nCarlOS> ");

    while (1)
    {

        if (keyboard_has_input())
        {
            char c = keyboard_get_char();

            if (c == KEY_ENTER)
            {
                print_newline();

                // Null-terminiere den Input
                input_buffer[buffer_pos] = '\0';

                // Verarbeite den Befehl
                shell_process_command(input_buffer);

                // Reset Buffer und zeige neuen Prompt
                buffer_pos = 0;
                print_str("CarlOS> ");
            }
            else if (c == KEY_BACKSPACE)
            {
                // Backspace behandeln
                if (buffer_pos > 0)
                {
                    buffer_pos--;
                    delete_char();
                }
            }
            else if (c == KEY_ESCAPE)
            {
                break;
            }
            else if (c >= 32 && c <= 126)
            { // Druckbare Zeichen
                // Prüfe Buffer-Überlauf
                if (buffer_pos < SHELL_BUFFER_SIZE - 1)
                {
                    input_buffer[buffer_pos] = c;
                    buffer_pos++;
                    print_char(c);
                }
            }
        }
    }
}

// COMMAND IMPLEMENTATIONS

void cmd_about(void)
{
    print_clear();
    print_logo();
    print_set_color(forground_color, background_color);
    print_str("\n");
    print_str("     Name:        CarlOS\n");
    print_str("     Version:     1.0\n");
    print_str("     By:  CarlosFuessler\n");
    print_str("     Architectur: x86_64\n\n");
    print_calc();
}

void cmd_help(void)
{
    print_clear();
    print_str("=== Commands ===\n\n");
    print_str("about  - Shows Infos about CarlOS\n");
    print_str("help   - Shows this help\n");
    print_str("clear  - Clears the screen\n");
    print_str("color  - Lets you change the system font\n");
    print_str("echo   -Lets you write some text in the shell\n");
    print_str("\nUse ESC to leave the shell\n");
}

void switch_color()
{
    print_clear();
    print_str("=== Choose a color ===\n\n");
    print_str("Blue (1):\n");
    print_str("Green (2):\n");
    print_str("Pink (3):\n");
    print_str("White (4):\n");
    print_str("Red (5):\n");

    while (1)
    {
        char input = keyboard_get_char();

        if (input == '1')
        {
            forground_color = PRINT_COLOR_LIGHT_BLUE;
            print_clear();
            print_set_color(forground_color, background_color);
            print_str("Your Systemcolor was succesfully changed\n");
            break;
        }
        else if (input == '2')
        {

            forground_color = PRINT_COLOR_GREEN;
            print_clear();
            print_set_color(forground_color, background_color);
            print_str("Your Systemcolor was succesfully changed\n");
            break;
        }
        else if (input == '3')
        {

            forground_color = PRINT_COLOR_PINK;
            print_clear();
            print_set_color(forground_color, background_color);
            print_str("Your Systemcolor was succesfully changed\n");
            break;
        }
        else if (input == '4')
        {

            forground_color = PRINT_COLOR_WHITE;
            print_clear();
            print_set_color(forground_color, background_color);
            print_str("Your Systemcolor was succesfully changed\n");
            break;
        }
        else if (input == '5')
        {

            forground_color = PRINT_COLOR_RED;
            print_clear();
            print_set_color(forground_color, background_color);
            print_str("Your Systemcolor was succesfully changed\n");
            break;
        }
        else
            print_clear();
        print_str("=== Choose a color ===\n\n");
        print_str("Blue (1):\n");
        print_str("Green (2):\n");
        print_str("Pink (3):\n");
        print_str("White (4):\n");
        print_str("Red (5):\n");
    }
}

void echo()
{
    print_clear();
    unsigned char input;
    print_str("===Write any Text you want! To close echo write $ === \n\n");

    do
    {
        input = keyboard_get_char();

        if (input == KEY_ENTER)
        {
            print_newline();
        }
        if (input == KEY_BACKSPACE && input_buffer[buffer_pos - 4] != '=')
        {
            delete_char();
        }

        if (input != KEY_BACKSPACE)
        {
            print_char(input);
        }

    } while (input != '$');
}