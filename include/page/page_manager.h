#pragma once

#include <stdio.h>
#include "stdint.h"
#include "page/page.h"
#include "util/result.h"

// in-memory objects
typedef struct {
    Page *pages;
} PageManager;

Result page_manager_new(Page *self);
Result page_manager_destroy(Page *self);

Result page_manager_write_value(PageManager *self, int32_t page_num, Value value);

Result page_manager_get_page_by_id(PageManager *self, size_t id, Page *page);

Result page_manager_read(PageManager *self, size_t page_id, size_t item_id, Value *data);


Result page_manager_get_free_page(PageManager *self, Page *page);

// Вопрос: где хранить item_id