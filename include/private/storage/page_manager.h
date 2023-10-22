#pragma once

#include "public/util/common.h"
#include "public/storage/page_manager.h"
#include "private/storage/page.h"

// in-memory objects
struct PageManager{
    FileManager *file_manager;
    AllocatedPage *allocated_pages;
    // represents amount of pages loaded in ram
    uint32_t allocated_pages_count;
    //TODO: check usage of pages_count.
    // keep track of free page
    Page* current_free_page;
};

// Only moves forward to next
struct PageIterator {
    PageManager *page_manager;
    page_index_t next_page_id;
    Page *current_page;
};

struct AllocatedPayload {
    uint8_t *payload;
    struct AllocatedPayload *next;
};

struct ItemIterator {
    PageIterator *page_iterator;
    item_index_t current_item_index;
    Item *current_item;
    uint32_t allocated_payloads_count;
    struct AllocatedPayload *allocated_payloads;
};

Result page_manager_page_new(PageManager *self, Page **page);
void page_manager_page_destroy(PageManager *self, Page *page);

uint32_t page_manager_get_page_size(PageManager *self);

uint32_t page_manager_get_pages_count(PageManager *self);

Result page_manager_set_pages_count(PageManager *self, uint32_t pages_count);

Page * page_manager_get_current_free_page(PageManager *self);

Result page_manager_set_current_free_page(PageManager *self, Page *page);

Result page_manager_read_page(PageManager *self, page_index_t id, Page **result_page);
Result page_manager_flush_page(PageManager *self, Page *page);

//page_index_t page_manager_get_next_page_id(PageManager *self);
// Вопрос: где хранить item_id

Result page_manager_get_page_from_ram(PageManager *self, page_index_t page_id, Page **result);

size_t page_manager_get_page_offset(PageManager *self, page_index_t page_id);

page_index_t page_manager_get_last_page_id(PageManager *self);

void page_manager_add_page_to_cache(PageManager *self, Page* page);

size_t convert_to_file_offset(PageManager *self, page_index_t page_id, size_t offset_in_page);

uint8_t *item_iterator_allocate_payload(ItemIterator *self, size_t size);

void item_iterator_free_payloads(ItemIterator *self);

Result page_manager_calculate_large_item_size(PageManager *self, Page *page, item_index_t item_id, uint32_t *result);

Page *page_manager_allocate_page(PageManager *self, page_index_t id);
void page_manager_free_pages(PageManager *self);
