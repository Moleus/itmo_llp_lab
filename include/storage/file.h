#pragma once

#include <stdbool.h>
#include "util/common.h"

typedef struct {
    FILE *file;
    bool is_open;
} FileState;

Result file_new(FileState *fs);

Result file_destroy(FileState *fs);

Result file_open(FileState *fs, char *filename);

Result file_close(FileState *fs);

Result file_write(FileState *fs, void *data, size_t offset, size_t size);

Result file_read(FileState *fs, void *data, size_t offset, size_t size);
