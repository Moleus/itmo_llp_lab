#pragma once

#include <stdio.h>
#include <stdlib.h>
#include "stdint.h"
#include "storage/page.h"
#include "util/common.h"
#include "storage/file_manager.h"

// in-memory objects
typedef struct {
// we can't store all pages in memory (>10GiB)
//    Page *pages;
    FileManager *file_manager;
    Page *pages;
    size_t pages_count;
} PageManager;

Result page_manager_new(PageManager *self, FileManager *file_manager);
Result page_manager_destroy(PageManager *self);

Result page_manager_page_new(PageManager *self, Page *page);
Result page_manager_page_destroy(PageManager *self, Page *page);

Result page_manager_get_page_by_id(PageManager *self, size_t id, Page *page);

Result page_manager_get_free_page(PageManager *self, Page *page);


// Private methods
int32_t page_manager_get_next_page_id(PageManager *self);

// Вопрос: где хранить item_id