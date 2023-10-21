#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "public/storage/page.h"

#define HEADER_SIZE sizeof(PageHeader)

typedef struct item_index_t {
    int32_t id;
} item_index_t;

static inline item_index_t item_id(int32_t value) {
    return (item_index_t) {.id = value};
}

static inline item_index_t next_item(item_index_t self) {
    return (item_index_t) {.id = self.id + 1};
}

//#define NULL_ITEM (Item){.size=0, .data = NULL, .index_in_page = 0, .is_deleted = true}

typedef struct page_index_t {
    int32_t id;
} page_index_t;
#define NULL_PAGE_INDEX (page_index_t) {.id = -1}

static inline page_index_t page_id(int32_t value) {
    return (page_index_t) {.id = value};
}

static inline page_index_t next_page(page_index_t self) {
    return (page_index_t) {.id = self.id + 1};
}

struct __attribute__((packed)) PageHeader {
    page_index_t page_id;
    uint32_t page_size;
    // offset of this page in file
    item_index_t next_item_id;
    uint32_t items_count;
    // these two fields describe a range of free space [start; end]
    // TODO: check that number doesn't overflow on subtraction
    uint32_t free_space_start_offset;
    uint32_t free_space_end_offset;
};

typedef struct PageMetaInfo PageMetaInfo;

struct PageMetaInfo{
    Page *rawPage;
    PageMetaInfo *next_page;
};

struct __attribute__((packed)) Page {
    PageHeader page_header;
    uint8_t page_payload[];
};

typedef struct {
    bool complete;
    uint32_t bytes_written;
} ItemWriteStatus;

// we store it on disk
typedef struct __attribute__((packed)) ItemMetadata {
    item_index_t item_id;
    uint32_t data_offset;
    uint32_t size;
    bool is_deleted;
    page_index_t continues_on_page;
} ItemMetadata;

// don't store on disk
struct Item {
    item_index_t index_in_page;
    bool is_deleted;
    ItemPayload payload;
};

typedef struct {
    uint32_t metadata_offset_in_page;
    ItemMetadata metadata;
    ItemWriteStatus write_status;
} ItemAddResult;

Page *page_new(page_index_t page_id, uint32_t page_size);

void page_destroy(Page *self);

// payload size
uint32_t page_get_payload_size(uint32_t page_size);

Result page_add_item(Page *self, ItemPayload payload, page_index_t continues_on_page, ItemAddResult *item_add_result);

Result page_delete_item(Page *self, Item *item);

Result page_get_item(Page *self, item_index_t item_id, Item *item);

uint32_t page_get_free_space_left(Page *self);

uint32_t page_get_payload_available_space(Page *self);
