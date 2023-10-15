#pragma once

#include "public/util/common.h"
#include "stdint.h"

typedef struct __attribute__((packed)) ItemPayload {
    void *data;
    uint32_t size;
} ItemPayload;

typedef struct Item Item;

typedef struct PageHeader PageHeader;

typedef struct Page Page;

size_t page_size(Page *self);