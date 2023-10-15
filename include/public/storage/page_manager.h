#pragma once

#include <stdbool.h>
#include "public/util/common.h"
#include "public/storage/file_manager.h"
#include "public/storage/page.h"
#include "public/document_db/node.h"
#include "private/storage/page.h"

// PaqeManger - is a top-level structure which provides access to pages (only)
// All manipulations with pages should be done through PageManager
// PageManager is a cache so if the page doesn't exist in ram then it allocates memory
/*
 * iterator uses page_manager to iterate over to find page
 * when we need to add item we use iterator which iterates over all pages
 * if iterator doesn't find free space then it creates new one
 */
typedef struct PageManager PageManager;

PageManager * page_manager_new();

void page_manager_destroy(PageManager *self);

Result page_manager_init(PageManager *self, const char *filename, uint32_t page_size, int32_t file_signature);

typedef struct PageIterator PageIterator;

// TODO: make private
PageIterator *page_iterator_new(PageManager *self);

void page_iterator_destroy(PageIterator *self);

Result page_iterator_next(PageIterator *self, Page **result);

bool page_iterator_has_next(PageIterator *self);

typedef struct ItemIterator ItemIterator;

ItemIterator *item_iterator_new(PageManager *page_manager);

void item_iterator_destroy(ItemIterator *self);

Result item_iterator_next(ItemIterator *self, Item **result);

bool item_iterator_has_next(ItemIterator *self);

// TODO: can we make this one private?
PageIterator *page_manager_get_pages(PageManager *self);

ItemIterator *page_manager_get_items(PageManager *self);

Result page_manager_put_item(PageManager *self, Page *page, ItemPayload payload, ItemAddResult *item_add_result);

Result page_manager_delete_item(PageManager *self, Page *page, Item *item);

// free page management
Result page_manager_get_page_for_data(PageManager *self, ItemPayload data, Page **result);