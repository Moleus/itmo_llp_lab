#include "private/storage/page_manager.h"

Page *page_manager_allocate_page(PageManager *self) {
    ASSERT_ARG_NOT_NULL(self)

    Page *page = NULL;
    Result res = page_manager_page_new(self, &page);
    ABORT_IF_FAIL(res, "Failed to allocate page")
    return page;
}

void page_manager_free_page(PageManager *self, Page *page) {
    ASSERT_ARG_NOT_NULL(self)
    ASSERT_ARG_NOT_NULL(page)

    page_manager_page_destroy(self, page);
}