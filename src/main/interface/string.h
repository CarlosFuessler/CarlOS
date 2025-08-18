#pragma once

#include <stddef.h>

// String-Funktionen
size_t strlen(const char *str);
int strcmp(const char *str1, const char *str2);
char *_strcpy(char *dest, const char *src);
char *_strcat(char *dest, const char *src);

// Zahlen-Konvertierung (für Debug/Print)
void int_to_string(int value, char *buffer);
void hex_to_string(unsigned int value, char *buffer);
int _stoi(const char *str);
