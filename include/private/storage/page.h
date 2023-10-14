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
#define NULL_PAGE_INDEX (page_index_t) {.id = -1}

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

typedef struct {
    bool complete;
    int32_t bytes_left;
} ItemWriteStatus;

typedef struct ItemMetadata {
    item_index_t item_id;
    int32_t data_offset;
    int32_t size;
    page_index_t continues_on_page;
} ItemMetadata;

// Reference to an item payload in a file
struct Item {
    ItemPayload payload;
    item_index_t index_in_page;
    bool is_deleted;
};

typedef struct {
    int32_t metadata_offset_in_page;
    ItemMetadata metadata;
    ItemWriteStatus write_status;
} ItemAddResult;

Page * page_new(page_index_t page_id, int32_t page_size);

Result page_destroy(Page *self);

// payload size
int32_t page_get_payload_size(int32_t page_size);

Result page_add_item(Page *self, ItemPayload payload, ItemAddResult *item_add_result);

Result page_add_split_item_start(Page *self, ItemPayload payload, ItemAddResult *item_add_result);

Result page_add_split_item_end(Page *self, ItemPayload payload, ItemAddResult *item_add_result);

Result page_delete_item(Page *self, Item *item);

int32_t page_get_free_space_left(Page *self);

int32_t page_get_payload_available_space(Page *self);
