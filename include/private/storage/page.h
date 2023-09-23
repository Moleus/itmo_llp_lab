#pragma once

#include <stdbool.h>
#include "public/storage/page.h"

struct PageHeader {
    int32_t page_id;
    int32_t file_offset;
    int32_t items_count;
    // these two fields describe a range of free space [start; end]
    int32_t free_space_start_offset;
    int32_t free_space_end_offset;
    bool is_dirty; // we don't need to store this field in file
};

struct Page {
    PageHeader page_header;
    PagePayload page_payload;
};

typedef struct ItemMetadata {
    int32_t id;
    int32_t offset;
    int32_t size;
} ItemMetadata;
