#pragma once

#include "public/storage/page_manager.h"
#include "private/storage/page.h"

// in-memory objects
struct PageManager{
    FileManager *file_manager;
    int32_t page_size;
    Page *pages;
    // represents amount of pages loaded in ram
    int32_t pages_in_memory;
    //TODO: check usage of pages_count.
    // represents total amount of pages in file
    int32_t pages_count;
};

// Only moves forward to next
struct PageIterator {
    PageManager *page_manager;
    page_index_t next_page_id;
    Page *current_page;
};

struct ItemIterator{
    PageIterator *page_iterator;
    item_index_t current_item_index;
    Item *current_item;
};

Result page_manager_page_new(PageManager *self, Page **page);
Result page_manager_page_destroy(PageManager *self, Page *page);

Result page_manager_read_page(PageManager *self, page_index_t id, Page **result_page);
Result page_manager_flush_page(PageManager *self, Page *page);

page_index_t page_manager_get_next_page_id(PageManager *self);
// Вопрос: где хранить item_id

Result page_manager_get_page_from_ram(PageManager *self, page_index_t page_id, Page **result);