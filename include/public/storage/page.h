#pragma once

#include "public/util/common.h"

typedef struct item_index_t {
    u_int32_t id;
} item_index_t;

// Reference to an item payload in a file
typedef struct Item {
    void *data;
    int32_t size;
    item_index_t index_in_page;
    bool is_deleted;
} Item;

static inline item_index_t item_id(int32_t value) {
    return (item_index_t) {.id = value};
}

static inline item_index_t next_item(item_index_t self) {
    return (item_index_t) {.id = self.id + 1};
}

#define NULL_ITEM (Item){.size=0, .data = NULL, .index_in_page = 0, .is_deleted = true}

static inline bool is_null_item(Item item) {
    return item.data == NULL;
}

typedef struct PageHeader PageHeader;

/*
 * Small structure. Container headers. All user data is referenced via pointers
 */
typedef struct Page Page;

typedef struct PagePayload {
    u_int8_t *bytes;
} PagePayload;

typedef struct page_index_t {
    u_int32_t id;
} page_index_t;

static inline page_index_t page_id(int32_t value) {
    return (page_index_t) {.id = value};
}

static inline page_index_t next_page(page_index_t self) {
    return (page_index_t) {.id = self.id + 1};
}

#define NULL_PAGE (PagePayload){.bytes = NULL}

static inline bool is_null_page(PagePayload page) {
    return page.bytes == NULL;
}

Result page_new(page_index_t page_id, int32_t page_size, Page *self);

Result page_destroy(Page *self);

Result page_item_new(Item *self, size_t id, int32_t size, void *data);

Result page_item_destroy(Item *self);

// get position after the last item in page
size_t page_get_end_position(Page *self);

int32_t page_get_data_offset();

Result page_read_item(Page *self, item_index_t item_id, Item *item);

Result page_write(Page *self, Item *data);

size_t page_size(Page *self);

// TODO: implement item+metadata addition to page