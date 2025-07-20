#include "shell.h"
#include "keyboard.h"
#include "print.h"
#include "string.h"

// ========== GLOBALE VARIABLEN ==========
static char input_buffer[SHELL_BUFFER_SIZE];
static size_t buffer_pos = 0;
uint8_t forground_color = PRINT_COLOR_PINK;
uint8_t background_color = PRINT_COLOR_BLACK;

// ========== SHELL INITIALISIERUNG ==========
void shell_init(void)
{
    print_clear();
    print_set_color(forground_color, background_color);
    print_str("\n===CarlOS Shell v1.0 initialisiert===\n\n");
    print_str("Tippen Sie 'help' fuer verfuegbare Befehle\n\n");
}

// ========== COMMAND PROCESSING ==========
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
    else
    {
        print_str("Unbekannter Befehl: ");
        print_str(command);
        print_str("\nTippen Sie 'help' fuer verfuegbare Befehle.\n");
    }
}

// ========== SHELL HAUPTSCHLEIFE ==========
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
                print_str("\nShell beendet. Auf Wiedersehen!\n");
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

// ========== COMMAND IMPLEMENTATIONS ==========

void cmd_about(void)
{
    print_clear();
    print_str("=== CarlOS Information ===\n\n");
    print_str("Name:        CarlOS\n");
    print_str("Version:     1.0\n");
    print_str("Entwickler:  CarlosFuessler\n");
    print_str("Architektur: x86_64\n");
    print_str("Features:\n");
    print_str("- 32-bit zu 64-bit Long Mode Übergang\n");
    print_str("- VGA-Text-Modus-Ausgabe\n");
    print_str("- Grundlegende Print-Funktionen\n");
    print_str("- String Umwandlung\n");
    print_str("- Shell prompting\n");
}

void cmd_help(void)
{
    print_clear();
    print_str("=== Verfuegbare Befehle ===\n\n");
    print_str("about  - Zeigt Informationen über CarlOS\n");
    print_str("help   - Zeigt diese Hilfe\n");
    print_str("clear  - Loescht den Bildschirm\n");
    print_str("\nVerwenden Sie ESC um die Shell zu beenden.\n");
}

void switch_color()
{
    print_str("=== Wähle eine Farbe ===\n\n");
    print_str("Blau (1):\n");
    print_str("Grün (2):\n");

    while (1)
    {
        char input = keyboard_get_char();

        if (input == '1')
        {
            forground_color = PRINT_COLOR_BLUE;
            print_clear();
            print_set_color(forground_color, background_color);
            print_str("Ihre Systemfrabe wurde erfolgreich geändert!\n");
            break;
        }
        else if (input == '2')
        {

            forground_color = PRINT_COLOR_GREEN;
            print_clear();
            print_set_color(forground_color, background_color);
            print_str("Ihre Systemfrabe wurde erfolgreich geändert!\n");
            break;
        }
        else
            print_clear();
        print_str("=== Wähle eine Farbe ===\n\n");
        print_str("Blau (1):\n");
        print_str("Grün (2):\n");
    }
}