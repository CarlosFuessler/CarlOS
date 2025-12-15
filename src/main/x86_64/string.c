#include "string.h"

typedef bool;
#define true 1
#define false 0
size_t strlen(const char *str)
{
    size_t length = 0;
    while (str[length] != '\0')
    {
        length++;
    }
    return length;
}

int strcmp(const char *str1, const char *str2)
{
    size_t i = 0;
    while (str1[i] != '\0' && str2[i] != '\0')
    {
        if (str1[i] != str2[i])
        {
            return (str1[i] < str2[i]) ? -1 : 1;
        }
        i++;
    }

    if (str1[i] == str2[i])
        return 0;
    return (str1[i] == '\0') ? -1 : 1;
}

char *_strcpy(char *dest, const char *src)
{
    size_t i = 0;
    while (src[i] != '\0')
    {
        dest[i] = src[i];
        i++;
    }
    dest[i] = '\0';
    return dest;
}

char *_strcat(char *dest, const char *src)
{
    size_t dest_len = strlen(dest);
    size_t i = 0;

    while (src[i] != '\0')
    {
        dest[dest_len + i] = src[i];
        i++;
    }
    dest[dest_len + i] = '\0';
    return dest;
}

void int_to_string(int value, char *buffer)
{
    if (value == 0)
    {
        buffer[0] = '0';
        buffer[1] = '\0';
        return;
    }

    int is_negative = (value < 0);
    if (is_negative)
        value = -value;

    int index = 0;
    while (value > 0)
    {
        buffer[index++] = '0' + (value % 10);
        value /= 10;
    }

    if (is_negative)
        buffer[index++] = '-';
    buffer[index] = '\0';
    for (int i = 0; i < index / 2; i++)
    {
        char temp = buffer[i];
        buffer[i] = buffer[index - 1 - i];
        buffer[index - 1 - i] = temp;
    }
}

void hex_to_string(unsigned int value, char *buffer)
{
    const char hex_digits[] = "0123456789ABCDEF";
    buffer[0] = '0';
    buffer[1] = 'x';

    if (value == 0)
    {
        buffer[2] = '0';
        buffer[3] = '\0';
        return;
    }

    int index = 2;
    char temp[16];
    int temp_index = 0;

    while (value > 0)
    {
        temp[temp_index++] = hex_digits[value % 16];
        value /= 16;
    }

    // Reverse and copy
    for (int i = temp_index - 1; i >= 0; i--)
    {
        buffer[index++] = temp[i];
    }
    buffer[index] = '\0';
}

int _stoi(const char *str)
{
    if (str == NULL)
        return 0;

    int res = 0;
    bool negative = false;

    while (*str == ' ')
    {
        str++;
    }

    if (*str == '-')
    {
        negative = true;
        str++;
    }
    else if (*str == '+')
    {
        str++;
    }

    while (*str >= '0' && *str <= '9')
    {
        res = res * 10 + (*str - '0');
        str++;
    }

    return negative ? -res : res;
}
