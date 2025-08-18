#include "memory_management.h"

void *_memset(void *ptr, int value, size_t size)
{
    unsigned char *p = (unsigned char *)ptr;
    for (size_t i = 0; i < size; i++)
    {
        p[i] = (unsigned char)value;
    }
    return ptr;
}

// Speicher kopieren - HINZUGEFÜGT
void *_memcpy(void *dest, const void *src, size_t size)
{
    unsigned char *d = (unsigned char *)dest;
    const unsigned char *s = (const unsigned char *)src;

    for (size_t i = 0; i < size; i++)
    {
        d[i] = s[i];
    }

    return dest;
}

// Speicher vergleichen
int memcmp(const void *ptr1, const void *ptr2, size_t size)
{
    const unsigned char *p1 = (const unsigned char *)ptr1;
    const unsigned char *p2 = (const unsigned char *)ptr2;

    for (size_t i = 0; i < size; i++)
    {
        if (p1[i] < p2[i])
            return -1;
        if (p1[i] > p2[i])
            return 1;
    }

    return 0;
}