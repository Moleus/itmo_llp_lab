#include <assert.h>
#include "private/storage/page_manager.h"

#include "private/storage/page.h"
#include "private/storage/file_manager.h"

// Public
PageManager *page_manager_new(void) {
    PageManager *pm = malloc(sizeof(PageManager));
    ASSERT_NOT_NULL(pm, FAILED_TO_ALLOCATE_MEMORY)
    pm->file_manager = file_manager_new();
    pm->allocated_pages = NULL;
    pm->allocated_pages_count = 0;
    pm->current_free_page = NULL;
    return pm;
}

Result page_manager_init(PageManager *self, const char *filename, uint32_t page_size, int32_t file_signature) {
    ASSERT_ARG_NOT_NULL(self)

    LOG_DEBUG("PageManager - init. Struct sizes [ Page: %ld, NodeValue: %ld, ItemMetadata: %ld, Node: %ld, FileHeader: %ld, PageHeader: %ld", sizeof(Page), sizeof(NodeValue), sizeof(ItemMetadata), sizeof(Node),
              sizeof(FileHeader), sizeof(PageHeader));
    FileHeaderConstants header_for_new_file = {
            .signature = file_signature,
            .page_size = page_size
    };

    Result res = file_manager_init(self->file_manager, filename, header_for_new_file);
    RETURN_IF_FAIL(res, "Failed to read file header")

    // this works both with empty file and file which contains data
    page_index_t free_page_id = page_id(self->file_manager->header.dynamic.current_free_page);
    Page *current_free_page = NULL;
    if (page_manager_get_pages_count(self) == 0) {
        page_manager_page_new(self, &current_free_page);
    } else {
        page_manager_read_page(self, free_page_id, &current_free_page);
    }
    assert(current_free_page != NULL);
    self->current_free_page = current_free_page;

    LOG_DEBUG("Page manager initialized. Pages count: %d. Free page: %d, ", page_manager_get_pages_count(self), free_page_id.id);

    return OK;
}

void page_manager_destroy(PageManager *self) {
    ASSERT_ARG_NOT_NULL(self)

    file_manager_destroy(self->file_manager);
    page_manager_free_pages(self);
    free(self);
}

/*
 * Allocates new page
 */
Result page_manager_page_new(PageManager *self, Page **page) {
    ASSERT_ARG_NOT_NULL(self)
    ASSERT_ARG_IS_NULL(*page)

    page_index_t next_id = next_page(page_manager_get_last_page_id(self));
    *page = page_manager_allocate_page(self, next_id);
    uint32_t old_pages_count = page_manager_get_pages_count(self);
    Result res = page_manager_set_pages_count(self, ++old_pages_count);
    LOG_DEBUG("New page %d, total pages count: %d", next_id.id, page_manager_get_pages_count(self));
    RETURN_IF_FAIL(res, "Failed increment pages count")
    // write to file
    res = page_manager_write_page_on_disk(self, *page);
    RETURN_IF_FAIL(res, "Failed to write new page to file")

    return OK;
}

Result page_manager_flush_page(PageManager *self, Page *page) {
    ASSERT_ARG_NOT_NULL(self)
    ASSERT_ARG_NOT_NULL(page)

    size_t offset_in_file = page_manager_get_page_offset(self, page->page_header.page_id);
    size_t size = page_size(page);

    file_manager_write(self->file_manager, offset_in_file, size, page);
    return OK;
}

// Private
Result page_manager_get_page_from_ram(PageManager *self, page_index_t page_id, Page **result) {
    ASSERT_ARG_NOT_NULL(self)
    ASSERT_ARG_IS_NULL(*result)

    // for each page in pages
    AllocatedPage *current_page = self->allocated_pages;
    for (uint32_t i = 0; i < self->allocated_pages_count; i++) {
        if (current_page == NULL) {
            LOG_ERR("Page from ram %d is null. Pages in memory: %d", i, self->allocated_pages_count);
            ABORT_EXIT(INTERNAL_LIB_ERROR, "Page in memory is null")
        }
        if (current_page->rawPage->page_header.page_id.id == page_id.id) {
            *result = current_page->rawPage;
            return OK;
        }
        current_page = current_page->next;
    }
    return ERROR("Page not found in ram");
}
