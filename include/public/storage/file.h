#pragma once

#include "public/util/common.h"

typedef struct FileState FileState;

Result file_new(FileState **fs);

Result file_destroy(FileState *fs);

Result file_open(FileState *fs, const char *filename);

Result file_close(FileState *fs);

Result file_write(FileState *fs, void *data, size_t offset, size_t size);

Result file_read(FileState *fs, void *data, size_t offset, size_t size);

bool file_is_open(FileState *fs);