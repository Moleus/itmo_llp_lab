#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "public/storage/file.h"

struct FileState {
    FILE *file;
    bool is_open;
    bool is_new;
    uint32_t size;
};
