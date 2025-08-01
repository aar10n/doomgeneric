//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
// Copyright(C) 2025 Aaron Gill-Braun
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// DESCRIPTION:
//	WAD I/O functions.
//

#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/fcntl.h>
#include <sys/stat.h>

#include "m_misc.h"
#include "w_file.h"
#include "z_zone.h"

typedef struct
{
  wad_file_t wad;
  int fd;
} posix_wad_file_t;

extern wad_file_class_t posix_wad_file;

static wad_file_t *W_Posix_OpenFile(char *path)
{
  posix_wad_file_t *result;

  int fd = open(path, O_RDONLY);
  if (fd < 0)
  {
    return NULL;
  }

  // Determine the size of the file.
  struct stat file_stat;
  if (fstat(fd, &file_stat) < 0)
  {
    close(fd);
    return NULL;
  }

  // Map the file into memory.
  void *mapped = mmap(NULL, file_stat.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
  if (mapped == MAP_FAILED)
  {
    close(fd);
    return NULL;
  }

  // Create a new posix_wad_file_t to hold the file handle.

  result = Z_Malloc(sizeof(posix_wad_file_t), PU_STATIC, 0);
  result->wad.file_class = &posix_wad_file;
  result->wad.mapped = mapped;
  result->wad.length = file_stat.st_size;
  result->fd = fd;

  return &result->wad;
}

static void W_Posix_CloseFile(wad_file_t *wad)
{
  posix_wad_file_t *posix_wad;

  posix_wad = (posix_wad_file_t *) wad;

  munmap(posix_wad->wad.mapped, posix_wad->wad.length);

  close(posix_wad->fd);
  Z_Free(posix_wad);
}

// Read data from the specified position in the file into the
// provided buffer.  Returns the number of bytes read.

size_t W_Posix_Read(wad_file_t *wad, unsigned int offset,
                   void *buffer, size_t buffer_len)
{
  posix_wad_file_t *posix_wad;
  size_t result;

  posix_wad = (posix_wad_file_t *) wad;

  // Jump to the specified position in the file.

  lseek(posix_wad->fd, offset, SEEK_SET);

  // Read into the buffer.

  result = read(posix_wad->fd, buffer, buffer_len);

  return result;
}


wad_file_class_t posix_wad_file =
  {
    W_Posix_OpenFile,
    W_Posix_CloseFile,
    W_Posix_Read,
  };
