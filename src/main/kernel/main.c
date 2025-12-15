#include "keyboard.h"
#include "print.h"
#include "shell.h"

void kernel_main(void) {
  print_clear();
  print_logo_welcome();
  print_str("Welcome to CarlOS!\n");
  print_str("Press $ to start the shell!\n\n");

  keyboard_init();
  char input;
  do {
    input = keyboard_get_char();
    print_char(input);
    print_newline();

    if (input != '<') {
      print_str("Wrong Input, please press <\n");
    }
  } while (input != '$');

  print_str("Starting shell...\n\n");

  shell_init();
  shell_run();

  print_clear();
  print_logo_goodbye();
  while (1) {
  }
}
