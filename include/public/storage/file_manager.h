#pragma once

#include "public/util/common.h"
#include <stdlib.h>

typedef struct FileHeader FileHeader;

typedef struct FileManager FileManager;

typedef struct FileHeaderConstants FileHeaderConstants;

FileManager * file_manager_new();

Result file_manager_destroy(FileManager *self);

Result file_manager_init(FileManager *self, const char *filename, FileHeaderConstants header_for_new_file);

// write file header to file
Result file_manager_write_header(FileManager *self);

Result file_manager_read(FileManager *self, size_t offset, size_t size, void *buffer);
Result file_manager_write(FileManager *self, size_t offset, size_t size, void *buffer);

Result file_manager_close(FileManager *self);