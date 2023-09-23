#pragma once

#include "public/util/common.h"

// stored in file
typedef struct Item {
    void *data;
    int32_t size;
} Item;

#define NULL_ITEM (Item){.size=0, .data = NULL}
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

#define NULL_PAGE (PagePayload){.bytes = NULL}
static inline bool is_null_page(PagePayload page) {
    return page.bytes == NULL;
}

Result page_new(int32_t page_id, Page *self);

Result page_destroy(Page *self);

Result page_item_new(Item *self, size_t id, int32_t size, void *data);

Result page_item_destroy(Item *self);

// get position after the last item in page
size_t page_get_end_position(Page *self);

int32_t page_get_data_offset();

Result page_read(Page *self, size_t item_id, Item *item);

Result page_write(Page *self, Item *data);

Result page_add_item(Page *self, Item *data);

size_t page_size();

// TODO: implement item+metadata addition to page