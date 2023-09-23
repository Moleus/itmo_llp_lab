#pragma once

#include "public/storage/file_manager.h"
#include "private/storage/file.h"

struct FileHeader {
    int32_t signature;
    size_t size;
    size_t first_free_page_id;
    size_t last_free_page_id;
    size_t page_count;
    size_t records_count;
};

// file object which stores pointer to file
struct FileManager{
    FileState *file;
    FileHeader header;
};
