#pragma once

#include "public/util/common.h"
#include "public/storage/file_manager.h"
#include "public/storage/page.h"

typedef struct PageManager PageManager;

Result page_manager_new(PageManager *self, FileManager *file_manager);
Result page_manager_destroy(PageManager *self);

Result page_manager_page_new(PageManager *self, Page *page);
Result page_manager_page_destroy(PageManager *self, Page *page);

Result page_manager_get_page_by_id(PageManager *self, size_t id, Page *page);

Result page_manager_get_free_page(PageManager *self, Page *page);