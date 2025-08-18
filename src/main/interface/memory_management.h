#pragma once

#include <stdint.h>
#include <stddef.h>

void *_memset(void *ptr, int value, size_t size);
void *_memcpy(void *dest, const void *src, size_t size);
int memcmp(const void *ptr1, const void *ptr2, size_t size);
