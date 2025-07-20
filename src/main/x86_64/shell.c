#include "shell.h"
#include "keyboard.h"
#include "print.h"
#include "string.h"

// ========== GLOBALE VARIABLEN ==========
static char input_buffer[SHELL_BUFFER_SIZE];
static size_t buffer_pos = 0;

// ========== SHELL INITIALISIERUNG ==========
void shell_init(void)
{
    print_clear();
    print_str("CarlOS Shell v1.0 initialisiert\n");
    print_str("Tippen Sie 'help' für verfügbare Befehle\n");
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
    else
    {
        print_str("Unbekannter Befehl: ");
        print_str(command);
        print_str("\nTippen Sie 'help' für verfügbare Befehle.\n");
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
    print_str("=== CarlOS Information ===\n");
    print_str("Name:        CarlOS\n");
    print_str("Version:     1.0\n");
    print_str("Entwickler:  Carlos\n");
    print_str("Architektur: x86_64\n");
    print_str("Features:\n");
    print_str("  - VGA Text Mode\n");
    print_str("  - Keyboard Input\n");
    print_str("  - Basic Shell\n");
    print_str("  - Memory Management\n");
    print_str("\nEin einfaches Lern-Betriebssystem!\n");
}

void cmd_help(void)
{
    print_str("=== Verfügbare Befehle ===\n");
    print_str("about  - Zeigt Informationen über CarlOS\n");
    print_str("help   - Zeigt diese Hilfe\n");
    print_str("clear  - Löscht den Bildschirm\n");
    print_str("\nVerwenden Sie ESC um die Shell zu beenden.\n");
}