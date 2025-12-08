#include "shell.h"
#include "keyboard.h"
#include "print.h"
#include "string.h"
#include "calc.h"
#include "graphics.h"
#include "memory_management.h"
#include "file_system.h"

static char input_buffer[SHELL_BUFFER_SIZE];
static size_t buffer_pos = 0;
uint8_t forground_color = PRINT_COLOR_PINK;
uint8_t background_color = PRINT_COLOR_BLACK;
void shell_init(void)
{
    fs_init();
    print_clear();
    print_set_color(forground_color, background_color);
    print_str("####################################\n");
    print_str("######## CarlOS Shell v1.0 #########\n");
    print_str("####################################\n\n");
    print_str("Wirte <help> for a list of all commands!\n\n");
}

void shell_process_command(const char *command)
{
    while (*command == ' ')
        command++;

    if (strlen(command) == 0)
    {
        return;
    }
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
    else if (strcmp(command, "calc") == 0)
    {
        cmd_calc();
    }
    else if (strcmp(command, "touch") == 0)
    {
        cmd_touch();
    }
    else if (strcmp(command, "write") == 0)
    {
        cmd_write();
    }
    else if (strcmp(command, "cat") == 0)
    {
        cmd_cat();
    }
    else if (strcmp(command, "ls") == 0)
    {
        cmd_ls();
    }
    else if (strcmp(command, "rm") == 0)
    {
        cmd_rm();
    }
    else

    {
        print_clear();
        print_str("unknown command: ");
        print_str(command);
        print_str("\nwrite <help> for unknown commands \n");
    }
}

void shell_run(void)
{
    buffer_pos = 0;
    print_str("\nCarlOS> ");

    while (1)
    {

        if (keyboard_has_input())
        {
            char c = keyboard_get_char();

            if (c == KEY_ENTER)
            {
                print_newline();
                input_buffer[buffer_pos] = '\0';
                shell_process_command(input_buffer);
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
}

void cmd_help(void)
{
    print_clear();
    print_str("=== Commands ===\n\n");
    print_str("about  - Shows Infos about CarlOS\n");
    print_str("help   - Shows this help\n");
    print_str("clear  - Clears the screen\n");
    print_str("color  - Lets you change the system font\n");
    print_str("echo   - Lets you write some text in the shell\n");
    print_str("calc   - Lets you open an calculator\n");
    print_str("touch  - Create a new file\n");
    print_str("write  - Write text to a file\n");
    print_str("cat    - Display file content\n");
    print_str("ls     - List all files\n");
    print_str("rm     - Delete a file\n");
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

void cmd_calc(void)
{
    calc_input();
}

void cmd_touch(void)
{
    print_str("Filename: ");
    char filename[MAX_FILENAME];
    size_t pos = 0;

    while (1)
    {
        char c = keyboard_get_char();

        if (c == KEY_ENTER)
        {
            print_newline();
            filename[pos] = '\0';
            break;
        }

        if (c == KEY_BACKSPACE && pos > 0)
        {
            pos--;
            delete_char();
        }
        else if (pos < MAX_FILENAME - 1 && c != KEY_BACKSPACE)
        {
            filename[pos++] = c;
            print_char(c);
        }
    }

    int result = fs_create(filename);
    if (result == 0)
    {
        print_str("File created: ");
        print_str(filename);
        print_newline();
    }
    else if (result == -2)
    {
        print_str("Error: File already exists\n");
    }
    else if (result == -3)
    {
        print_str("Error: No space left\n");
    }
    else
    {
        print_str("Error: Invalid filename\n");
    }
}

void cmd_write(void)
{
    print_str("Filename: ");
    char filename[MAX_FILENAME];
    size_t pos = 0;

    while (1)
    {
        char c = keyboard_get_char();

        if (c == KEY_ENTER)
        {
            print_newline();
            filename[pos] = '\0';
            break;
        }

        if (c == KEY_BACKSPACE && pos > 0)
        {
            pos--;
            delete_char();
        }
        else if (pos < MAX_FILENAME - 1 && c != KEY_BACKSPACE)
        {
            filename[pos++] = c;
            print_char(c);
        }
    }

    print_str("Content ($ to finish): ");
    char content[MAX_FILESIZE];
    pos = 0;

    while (1)
    {
        char c = keyboard_get_char();

        if (c == '$')
        {
            print_newline();
            content[pos] = '\0';
            break;
        }

        if (c == KEY_ENTER)
        {
            if (pos < MAX_FILESIZE - 1)
            {
                content[pos++] = '\n';
                print_newline();
            }
        }
        else if (c == KEY_BACKSPACE && pos > 0)
        {
            pos--;
            delete_char();
        }
        else if (pos < MAX_FILESIZE - 1 && c != KEY_BACKSPACE)
        {
            content[pos++] = c;
            print_char(c);
        }
    }

    int result = fs_write(filename, content);
    if (result == 0)
    {
        print_str("Content written to ");
        print_str(filename);
        print_newline();
    }
    else
    {
        print_str("Error: File not found\n");
    }
}

void cmd_cat(void)
{
    print_str("Filename: ");
    char filename[MAX_FILENAME];
    size_t pos = 0;

    while (1)
    {
        char c = keyboard_get_char();

        if (c == KEY_ENTER)
        {
            print_newline();
            filename[pos] = '\0';
            break;
        }

        if (c == KEY_BACKSPACE && pos > 0)
        {
            pos--;
            delete_char();
        }
        else if (pos < MAX_FILENAME - 1 && c != KEY_BACKSPACE)
        {
            filename[pos++] = c;
            print_char(c);
        }
    }

    char buffer[MAX_FILESIZE];
    int result = fs_read(filename, buffer, MAX_FILESIZE);

    if (result >= 0)
    {
        print_newline();
        print_str(buffer);
        print_newline();
    }
    else
    {
        print_str("Error: File not found\n");
    }
}

void cmd_ls(void)
{
    print_str("Files:\n");
    fs_list();
}

void cmd_rm(void)
{
    print_str("Filename: ");
    char filename[MAX_FILENAME];
    size_t pos = 0;

    while (1)
    {
        char c = keyboard_get_char();

        if (c == KEY_ENTER)
        {
            print_newline();
            filename[pos] = '\0';
            break;
        }

        if (c == KEY_BACKSPACE && pos > 0)
        {
            pos--;
            delete_char();
        }
        else if (pos < MAX_FILENAME - 1 && c != KEY_BACKSPACE)
        {
            filename[pos++] = c;
            print_char(c);
        }
    }

    int result = fs_delete(filename);
    if (result == 0)
    {
        print_str("File deleted: ");
        print_str(filename);
        print_newline();
    }
    else
    {
        print_str("Error: File not found\n");
    }
}
