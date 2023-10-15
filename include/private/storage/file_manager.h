#pragma once

#include "public/storage/file_manager.h"
#include "private/storage/file.h"
#include "stdint.h"

struct __attribute__((packed)) FileHeaderConstants {
    int32_t signature;
    uint32_t page_size;
};

typedef struct __attribute__((packed)) {
    uint32_t file_size;
    int32_t current_free_page;
    uint32_t page_count;
} FileHeaderDynamic;

struct __attribute__((packed)) FileHeader {
    FileHeaderConstants constants;
    FileHeaderDynamic dynamic;
};

// file object which stores pointer to file
struct FileManager {
    FileState *file;
    FileHeader header;
};

// read file header from file
Result file_manager_read_header(FileManager *self);

Result file_manager_open(FileManager *self, const char *filename);
