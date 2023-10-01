#pragma once

#include "public/storage/page_manager.h"
#include "private/storage/page.h"

// in-memory objects
struct PageManager{
// we can't store all pages in memory (>10GiB)
//    Page *pages;
    FileManager *file_manager;
    size_t page_size;
    Page *pages;
    size_t pages_count;
};

// Only moves forward to next
struct PageIterator {
    PageManager *page_manager;
    page_index_t next_page_id;
    Page current_page;
};

struct ItemIterator{
    PageIterator *page_iterator;
    item_index_t current_item_index;
    Item current_item;
};

page_index_t page_manager_get_next_page_id(PageManager *self);
// Вопрос: где хранить item_id

Result page_manager_get_page_from_ram(PageManager *self, page_index_t page_id, Page *result);