#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "public/storage/file.h"

#ifdef _WIN32
#include <io.h>
#define F_OK 0
#define access _access
#else
#include <unistd.h>
#endif


struct FileState {
    FILE *file;
    bool is_open;
    bool is_new;
    uint32_t size;
};
