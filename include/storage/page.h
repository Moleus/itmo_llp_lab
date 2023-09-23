#pragma once

#include "util/result.h"
#include <stdio.h>
#include <stdbool.h>
#include "stdint.h"

typedef struct {
    void *data;
    int32_t pos;
    int32_t len;
} Value;

// in file objects???
// private
typedef struct {
    Value data;
    size_t id;
} PageItem;

Result page_item_new(PageItem *self, size_t id, int32_t size, void *data);

Result page_item_destroy(PageItem *self);

// TODO: do we need to change data and size inside page item?
typedef enum {
    PAGE_STATUS_UNUSED,
    PAGE_STATUS_DELETED
} PageStatus;

typedef struct {
    int32_t used_bytes;
    PageStatus status;
} PageHeader;

// TODO: why do we need this
Result page_header_read(Value *data, size_t offset, PageHeader *header);
Result page_header_write(Value *data, size_t offset, PageHeader *header);

// How to access file data from page functions?
// we need to pass file wrapper to page functions
typedef struct {
    PageHeader page_header;
    // offset from the start of the file where page starts
    int32_t offset;
    int32_t page_id;
} Page;

// get position after the last item in page
size_t page_get_end_position(Page *self);

Result page_new(int32_t page_id, Page *self);
Result page_destroy(Page *self);

int32_t page_get_data_offset();

Result page_read(Page *self, size_t item_id, PageItem *item);

Result page_write(Page *self, Value *data);

size_t page_size();
