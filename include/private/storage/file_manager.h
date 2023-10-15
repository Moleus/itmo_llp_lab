#pragma once

#include "public/storage/file_manager.h"
#include "private/storage/file.h"
#include "stdint.h"

struct FileHeaderConstants{
    int32_t signature;
    uint32_t page_size;
} ;

typedef struct {
    uint32_t file_size;
    uint32_t current_free_page;
    uint32_t page_count;
} FileHeaderDynamic;

struct FileHeader {
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
