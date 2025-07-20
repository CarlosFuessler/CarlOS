#pragma once

#include <stddef.h>

// Grundlegende String-Funktionen
size_t strlen(const char *str);
int strcmp(const char *str1, const char *str2);
char *strcpy(char *dest, const char *src);
char *strcat(char *dest, const char *src);

// Zahlen-Konvertierung (für Debug/Print)
void int_to_string(int value, char *buffer);
void hex_to_string(unsigned int value, char *buffer);

// Speicher-Funktionen
void *memset(void *ptr, int value, size_t size);
void *memcpy(void *dest, const void *src, size_t size);
int memcmp(const void *ptr1, const void *ptr2, size_t size); // <- Hinzugefügt