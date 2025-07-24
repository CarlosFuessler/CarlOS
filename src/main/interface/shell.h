#pragma once

#include <stdint.h>
#include <stddef.h>

#define SHELL_BUFFER_SIZE 128

// Shell-Funktionen
void shell_init(void);
void shell_run(void);
void shell_process_command(const char *command);

// Command-Funktionen
void cmd_about(void);
void cmd_help(void);
void cmd_calc(void);