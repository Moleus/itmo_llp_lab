#pragma once

#include <stdbool.h>
#include "public/storage/page.h"

typedef struct item_index_t {
    u_int32_t id;
} item_index_t;

static inline item_index_t item_id(int32_t value) {
    return (item_index_t) {.id = value};
}

static inline item_index_t next_item(item_index_t self) {
    return (item_index_t) {.id = self.id + 1};
}

//#define NULL_ITEM (Item){.size=0, .data = NULL, .index_in_page = 0, .is_deleted = true}

typedef struct page_index_t {
    u_int32_t id;
} page_index_t;

static inline page_index_t page_id(int32_t value) {
    return (page_index_t) {.id = value};
}

static inline page_index_t next_page(page_index_t self) {
    return (page_index_t) {.id = self.id + 1};
}

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

struct PagePayload {
    u_int8_t *bytes;
};

static inline bool is_null_page(PagePayload page) {
    return page.bytes == NULL;
}

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

// Reference to an item payload in a file
struct Item {
    ItemPayload payload;
    item_index_t index_in_page;
    bool is_deleted;
};

typedef struct ItemResult ItemResult;
struct ItemResult {
    int32_t metadata_offset_in_page;
    ItemMetadata metadata;
    item_index_t item_id;
};

Page * page_new(page_index_t page_id, int32_t page_size);

Result page_destroy(Page *self);

// payload size
int32_t page_get_payload_size(int32_t page_size);

Result page_add_item(Page *self, ItemPayload payload, struct ItemResult *item_add_result);

Result page_delete_item(Page *self, Item *item);

