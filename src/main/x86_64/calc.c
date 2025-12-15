

#include "calc.h"
#include "keyboard.h"
#include "memory_management.h"
#include "print.h"
#include "string.h"

#include <stdint.h>

struct equation {
  uint8_t number[16];
  uint8_t operation; // 1=+, 2=-, 3=*, 4=/
};

void calc_input(void) {
  struct equation terms[100];
  unsigned char input = 0;
  uint8_t term_index = 0;
  uint8_t digit_index = 0;

  print_clear();
  print_str("##########################\n");
  print_str("####### Calculator #######\n");
  print_str("##########################\n\n");

  print_set_color(PRINT_COLOR_WHITE, PRINT_COLOR_BLACK);
  print_str("Enter numbers and operations (= to calculate)\n");

  while (term_index < 99) {

    print_str("Number ");
    char num_str[8];
    int_to_string(term_index + 1, num_str);
    print_str(num_str);
    print_str(": ");

    digit_index = 0;

    while (1) {
      input = keyboard_get_char();

      if (input == '=') {
        if (digit_index > 0) {
          terms[term_index].number[digit_index] = '\0';
          term_index++;
        }
        goto calculate;
      }

      if (input == KEY_ENTER && digit_index > 0) {
        terms[term_index].number[digit_index] = '\0';
        print_newline();
        break;
      }

      if (input >= '0' && input <= '9' && digit_index < 15) {
        terms[term_index].number[digit_index] = input;
        digit_index++;
        print_char(input);
      }
    }

    print_str("Operation (1=+ , 2=- , 3=* , 4=/ , = (finish): ");

    while (1) {
      input = keyboard_get_char();

      if (input == '=') {
        term_index++;
        goto calculate;
      }

      if (input >= '1' && input <= '4') {
        terms[term_index].operation = input - '0';
        print_char(input);
        print_newline();
        break;
      }
    }

    term_index++;
  }

calculate:
  if (term_index == 0) {
    print_str("No numbers entered!\n");
    return;
  }

  print_str("\n");
  print_str("Calculation: ");
  int result = _stoi((char *)terms[0].number);
  print_str((char *)terms[0].number);

  for (int i = 1; i < term_index; i++) {
    int next_number = _stoi((char *)terms[i].number);

    switch (terms[i - 1].operation) {
    case 1:
      print_str(" + ");
      result += next_number;
      break;
    case 2:
      print_str(" - ");
      result -= next_number;
      break;
    case 3:
      print_str(" * ");
      result *= next_number;
      break;
    case 4:
      print_str(" / ");
      if (next_number != 0)
        result /= next_number;
      else {
        print_str("ERROR(div by 0)");
        return;
      }
      break;
    }

    print_str((char *)terms[i].number);
  }

  print_str(" = ");
  char result_str[16];
  int_to_string(result, result_str);
  print_str(result_str);
  print_str("\n\n");
}

void calc(void) {
  char buf_a[8], buf_b[8];
  calc_input();
}
