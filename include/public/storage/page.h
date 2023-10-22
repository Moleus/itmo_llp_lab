#pragma once

#include "public/util/common.h"
#include "stdint.h"

// we don't store it on disk
typedef struct ItemPayload {
    uint32_t size;
    void *data;
} ItemPayload;

typedef struct Item Item;

typedef struct PageHeader PageHeader;

typedef struct Page Page;

size_t page_size(Page *self);

bool page_can_fit_payload(Page *self, uint32_t payload_size);

bool page_can_fit_any_payload(Page *self);
