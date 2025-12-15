#pragma once

#include <stddef.h>

#define SHELL_BUFFER_SIZE 128

// Shell functions
void shell_init(void);
void shell_run(void);
void shell_process_command(const char *command);

// Command functions
void switch_color();
void echo();
void cmd_about(void);
void cmd_help(void);
void cmd_calc(void);
void cmd_touch(void);
void cmd_write(void);
void cmd_cat(void);
void cmd_ls(void);
void cmd_rm(void);
