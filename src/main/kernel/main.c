#include "print.h"
#include "keyboard.h"
#include "shell.h"

void kernel_main(void)
{
    // System initialisieren
    print_clear();
    print_set_color(PRINT_COLOR_YELLOW, PRINT_COLOR_BLACK);
    print_str("Welcome to CarlOS!\n");
    print_str("Druecken Sie < um die Shell zu starten!\n\n");

    // Keyboard initialisieren
    keyboard_init();

    // Warte auf < Taste
    char input = 0;
    while (input != '<')
    { // oder != KEY_LESS_THAN
        if (keyboard_has_input())
        {
            input = keyboard_get_char();

            if (input == '<')
            { // oder == KEY_LESS_THAN
                print_str("Shell wird gestartet...\n\n");
                break;
            }
            else
            {
                print_str("Falsche Taste! Druecken Sie <\n");
            }
        }
    }

    // Shell initialisieren und starten
    shell_init();
    shell_run();

    // Falls Shell beendet wird
    print_str("System wird heruntergefahren...\n");
    while (1)
    {
        // Halt
    }
}