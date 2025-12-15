#include "file_system.h"
#include "disk.h"
#include "memory_management.h"
#include "print.h"
#include "string.h"

static file_t files[MAX_FILES];

void fs_init(void)
{
  disk_init();

  uint8_t sector_buffer[SECTOR_SIZE];

  // Load sector 0
  if (disk_read_sector(0, sector_buffer) == 0)
  {
    // Check magic number
    if (sector_buffer[0] == 'C' && sector_buffer[1] == 'F' &&
        sector_buffer[2] == 'S' && sector_buffer[3] == '1')
    {
      // Load files from disk
      for (int i = 0; i < MAX_FILES; i++)
      {
        int sector = 1 + (i * 2);
        if (disk_read_sector(sector, sector_buffer) == 0)
        {
          _memcpy(&files[i], sector_buffer, sizeof(file_t));
        }
      }
      return;
    }
  }

  // Init new Filesystem
  for (int i = 0; i < MAX_FILES; i++)
  {
    files[i].used = 0;
    files[i].size = 0;
    _memset(files[i].name, 0, MAX_FILENAME);
    _memset(files[i].content, 0, MAX_FILESIZE);
  }

  // Magic Number
  _memset(sector_buffer, 0, SECTOR_SIZE);
  sector_buffer[0] = 'C';
  sector_buffer[1] = 'F';
  sector_buffer[2] = 'S';
  sector_buffer[3] = '1';
  disk_write_sector(0, sector_buffer);
}

static int find_file(const char *filename)
{
  for (int i = 0; i < MAX_FILES; i++)
  {
    if (files[i].used && strcmp(files[i].name, filename) == 0)
    {
      return i;
    }
  }
  return -1;
}

static int find_free_slot(void)
{
  for (int i = 0; i < MAX_FILES; i++)
  {
    if (!files[i].used)
    {
      return i;
    }
  }
  return -1;
}

static void fs_sync(void)
{
  uint8_t sector_buffer[SECTOR_SIZE];

  for (int i = 0; i < MAX_FILES; i++)
  {
    int sector = 1 + (i * 2);
    _memset(sector_buffer, 0, SECTOR_SIZE);
    _memcpy(sector_buffer, &files[i], sizeof(file_t));
    disk_write_sector(sector, sector_buffer);
  }
}

int fs_create(const char *filename)
{
  if (strlen(filename) >= MAX_FILENAME)
  {
    return -1;
  }

  if (find_file(filename) != -1)
  {
    return -2;
  }

  int slot = find_free_slot();
  if (slot == -1)
  {
    return -3;
  }

  _strcpy(files[slot].name, filename);
  files[slot].size = 0;
  files[slot].used = 1;
  _memset(files[slot].content, 0, MAX_FILESIZE);

  fs_sync();

  return 0;
}

int fs_write(const char *filename, const char *content)
{
  int idx = find_file(filename);
  if (idx == -1)
  {
    return -1;
  }

  size_t len = strlen(content);
  if (len >= MAX_FILESIZE)
  {
    len = MAX_FILESIZE - 1;
  }

  _memcpy(files[idx].content, content, len);
  files[idx].content[len] = '\0';
  files[idx].size = len;

  fs_sync();

  return 0;
}

int fs_read(const char *filename, char *buffer, size_t size)
{
  int idx = find_file(filename);
  if (idx == -1)
  {
    return -1;
  }

  size_t copy_size = files[idx].size;
  if (copy_size >= size)
  {
    copy_size = size - 1;
  }

  _memcpy(buffer, files[idx].content, copy_size);
  buffer[copy_size] = '\0';

  return copy_size;
}

int fs_delete(const char *filename)
{
  int idx = find_file(filename);
  if (idx == -1)
  {
    return -1;
  }

  files[idx].used = 0;
  files[idx].size = 0;
  _memset(files[idx].name, 0, MAX_FILENAME);
  _memset(files[idx].content, 0, MAX_FILESIZE);

  fs_sync();

  return 0;
}

void fs_list(void)
{
  int found = 0;
  for (int i = 0; i < MAX_FILES; i++)
  {
    if (files[i].used)
    {
      found = 1;
      print_str("  ");
      print_str(files[i].name);
      print_str(" (");
      char size_str[8];
      int_to_string(files[i].size, size_str);
      print_str(size_str);
      print_str(" bytes)\n");
    }
  }

  if (!found)
  {
    print_str("  (no files)\n");
  }
}
