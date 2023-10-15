#pragma once

#include "public/util/common.h"
#include "stdint.h"

typedef struct ItemPayload {
    void *data;
    uint32_t size;
} ItemPayload;

typedef struct Item Item;

typedef struct PageHeader PageHeader;

typedef struct Page Page;

typedef struct PagePayload PagePayload;

size_t page_size(Page *self);