#pragma once
// storage - interface to add/update/delete items in memory. It accepts *void raw_data and
// calls methods from page_manager/file_manager to persist item in db.

#include "private/storage/page_manager.h"

typedef struct {
    PageManager *page_manager;
} Storage;


Storage* storage_new(PageManager *page_manager);


Result add_item(Storage *self, void *raw_data, size_t size);

Result update_item(Storage *self, void *raw_data, size_t size);

Result delete_item(Storage *self, void *raw_data, size_t size);

Result get_item(Storage *self, void *raw_data, size_t size);