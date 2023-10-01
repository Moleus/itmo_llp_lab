#pragma once

#include <stdbool.h>
#include "public/storage/page.h"

struct PageHeader {
    page_index_t page_id;
    int32_t page_size;
    // offset of this page in file
    int32_t file_offset;
    item_index_t next_item_id;
    // these two fields describe a range of free space [start; end]
    int32_t free_space_start_offset;
    int32_t free_space_end_offset;
    bool is_dirty; // we don't need to store this field in file
};

typedef struct PageInMemoryData PageInMemoryData;
struct PageInMemoryData {
   Page* next_page;
};

struct Page {
    PageHeader page_header;
    PageInMemoryData page_metadata;
    PagePayload page_payload;
};

typedef struct ItemMetadata {
    item_index_t id;
    int32_t data_offset;
    int32_t size;
} ItemMetadata;

typedef struct ItemResult ItemResult;
struct ItemResult {
    int32_t metadata_offset_in_page;
    ItemMetadata metadata;
    item_index_t item_id;
};

// payload size
int32_t page_get_payload_size(int32_t page_size);


Result page_add_item(Page *self, Item *item, ItemResult* item_add_result);

Result page_delete_item(Page *self, Item *item);