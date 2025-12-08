#pragma once

#include <stddef.h>
#include <stdint.h>

#define MAX_FILES 10
#define MAX_FILENAME 16
#define MAX_FILESIZE 256

typedef struct
{
    char name[MAX_FILENAME];
    char content[MAX_FILESIZE];
    size_t size;
    uint8_t used;
} file_t;

void fs_init(void);
int fs_create(const char *filename);
int fs_write(const char *filename, const char *content);
int fs_read(const char *filename, char *buffer, size_t size);
int fs_delete(const char *filename);
void fs_list(void);
