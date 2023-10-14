#pragma once

#include "public/util/common.h"
#include <stdlib.h>

typedef struct FileHeader FileHeader;

typedef struct FileManager FileManager;

FileManager * file_manager_new();

Result file_manager_destroy(FileManager *self);

Result file_manager_open(FileManager *self, char *filename);

// read file header from file
Result file_manager_read_header(FileManager *self, FileHeader *header);

// write file header to file
Result file_manager_write_header(FileManager *self, FileHeader *header);

Result file_manager_read(FileManager *self, size_t offset, size_t size, void *buffer);
Result file_manager_write(FileManager *self, size_t offset, size_t size, void *buffer);

Result file_manager_close(FileManager *self);