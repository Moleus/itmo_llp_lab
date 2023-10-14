#pragma once

#include "public/storage/file_manager.h"
#include "private/storage/file.h"
#include "stdint.h"

struct FileHeaderConstants {
    int32_t signature;
    uint32_t page_size;
};

struct FileHeaderDynamic {
    uint32_t file_size;
    uint32_t current_free_page;
    uint32_t page_count;
};

struct FileHeader {
    struct FileHeaderConstants constants;
    struct FileHeaderDynamic dynamic;
};

// file object which stores pointer to file
struct FileManager{
    FileState *file;
    FileHeader header;
};
