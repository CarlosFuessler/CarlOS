#include "print.h"
#include "keyboard.h"
#include "shell.h"

void kernel_main(void)
{
    // System initialisieren
    print_clear();
    print_set_color(PRINT_COLOR_YELLOW, PRINT_COLOR_BLACK);
    print_str("Welcome to CarlOS!\n");
    print_str("Press $ to start the shell!\n\n");

    // Keyboard initialisieren
    keyboard_init();

    // Warte auf < Taste (blockierend)
    char input;
    do
    {
        input = keyboard_get_char();
        print_char(input);
        print_newline();

        if (input != '<')
        {
            print_str("Falsche Taste! Druecken Sie <\n");
        }
    } while (input != '$');

    print_str("Shell wird gestartet...\n\n");

    // Shell initialisieren und starten
    shell_init();
    shell_run();

    print_clear();

    // Falls Shell beendet wird
    print_logo_goodbye();
    while (1)
    {
        // Halt
    }
}