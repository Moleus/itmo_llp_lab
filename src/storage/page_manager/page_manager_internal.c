#include "private/storage/page_manager.h"

#include "private/storage/page.h"
#include "private/storage/file_manager.h"

Page *page_manager_allocate_page(PageManager *self, page_index_t id) {
    uint32_t page_size = page_manager_get_page_size(self);
    Page *page = page_new(id, page_size);

    struct AllocatedPage *allocated_page = calloc(1, sizeof(struct AllocatedPage));
    ASSERT_NOT_NULL(allocated_page, FAILED_TO_ALLOCATE_MEMORY)

    allocated_page->rawPage = page;
    allocated_page->next = self->allocated_pages;
    self->allocated_pages = allocated_page;
    self->allocated_pages_count++;

    return page;
}

void page_manager_free_pages(PageManager *self) {
    ASSERT_ARG_NOT_NULL(self)

    uint32_t i = 0;
    struct AllocatedPage *allocated_pages = self->allocated_pages;
    while (allocated_pages != NULL) {
        i++;
        struct AllocatedPage *next = allocated_pages->next;
        page_destroy(allocated_pages->rawPage);
        free(allocated_pages);
        allocated_pages = next;
    }
    LOG_DEBUG("PageManager - freed %d pages. Count was %d", i, self->allocated_pages_count);
    self->allocated_pages_count = 0;
    self->allocated_pages = NULL;
    self->current_free_page = NULL;
}

Result page_manager_write_page_on_disk(PageManager *self, Page *page) {
    ASSERT_ARG_NOT_NULL(self)
    ASSERT_ARG_NOT_NULL(page)

    uint32_t page_size = page_manager_get_page_size(self);
    uint32_t page_offset_in_file = page_manager_get_page_offset(self, page->page_header.page_id);
    Result page_write_res = file_manager_write(self->file_manager, page_offset_in_file, page_size,
                                               page);
    RETURN_IF_FAIL(page_write_res, "Failed to write page on disk")
    LOG_DEBUG("Written page %d bytes %d to offset %08X", page->page_header.page_id.id, page_size, page_offset_in_file);

    return OK;
}
/*
 * Page should exist in memory or on disk
 * If page doesn't exist in memory then it will be loaded from disk
 * This is the only method which loads page from disk
 */
Result page_manager_read_page(PageManager *self, page_index_t id, Page **result_page) {
    ASSERT_ARG_NOT_NULL(self)
    ASSERT_ARG_IS_NULL(*result_page)

    // NOTE: здесь мы выдаем ошибку, если страницы не существует.
    uint32_t pages_count = page_manager_get_pages_count(self);
    if (id.id >= (int32_t) pages_count) {
        LOG_ERR("Page %d doesn't exist. Total pages: %d", id.id, pages_count);
        ABORT_EXIT(INTERNAL_LIB_ERROR, "Page doesn't exist")
    }
    Result res;

    // check if page exists in ram
    res = page_manager_get_page_from_ram(self, id, result_page);
    if (res.status == RES_OK) {
        LOG_INFO("Page %d found in ram", id.id);
        // page found in ram
        return OK;
    }

    // pass offset to file_manger and get page
    size_t page_offset_in_file = page_manager_get_page_offset(self, id);
    // load from disk
    // allocate page here
    Page *page = page_manager_allocate_page(self, id);
    // read header
    uint32_t page_size = page_manager_get_page_size(self);
    res = file_manager_read(self->file_manager, page_offset_in_file, page_size, page);
    RETURN_IF_FAIL(res, "Failed to read page header from file")
    // read payload
    LOG_DEBUG("Read page %d. Offset: %08X, items count: %d, free space: %d", id.id,
              page_offset_in_file, page->page_header.items_count,
              page->page_header.free_space_end_offset - page->page_header.free_space_start_offset);

    *result_page = page;

    return OK;
}
