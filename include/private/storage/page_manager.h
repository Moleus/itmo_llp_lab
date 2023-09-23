#pragma once

#include "public/storage/page_manager.h"

// in-memory objects
struct PageManager{
// we can't store all pages in memory (>10GiB)
//    Page *pages;
    FileManager *file_manager;
    Page *pages;
    size_t pages_count;
};

int32_t page_manager_get_next_page_id(PageManager *self);
// Вопрос: где хранить item_id