#pragma once

#include <stdbool.h>
#include "public/util/common.h"
#include "public/storage/file_manager.h"
#include "public/storage/page.h"

// PaqeManger - is a top-level structure which provides access to pages (only)
// All manipulations with pages should be done through PageManager
// PageManager is a cache so if the page doesn't exist in ram then it allocates memory
/*
 * iterator uses page_manager to iterate over to find page
 * when we need to add item we use iterator which iterates over all pages
 * if iterator doesn't find free space then it creates new one
 */
typedef struct PageManager PageManager;

Result page_manager_new(PageManager *self, FileManager *file_manager);
Result page_manager_destroy(PageManager *self);

Result page_manager_page_new(PageManager *self, Page *page);
Result page_manager_page_destroy(PageManager *self, Page *page);

Result page_manager_read_page(PageManager *self, page_index_t id, Page **result_page);
Result page_manager_flush_page(PageManager *self, Page *page);

typedef struct PageIterator PageIterator;

// TODO: make private
Result page_iterator_new(PageManager *page_manager, PageIterator **result);

void page_iterator_destroy(PageIterator *self);

Result page_iterator_next(PageIterator *self, Page **result);

bool page_iterator_has_next(PageIterator *self);

typedef struct ItemIterator ItemIterator;

Result item_iterator_new(PageManager *page_manager, ItemIterator **result);

void item_iterator_destroy(ItemIterator *self);

Result item_iterator_next(ItemIterator *self, Item **result);

bool item_iterator_has_next(ItemIterator *self);

// TODO: can we make this one private?
Result page_manager_get_pages(PageManager *self, PageIterator **result);

Result page_manager_get_items(PageManager *self, ItemIterator **result);