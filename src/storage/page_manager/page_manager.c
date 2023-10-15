#include "private/storage/page_manager.h"

//TODO: think if it's correct to include private header
#include "private/storage/page.h"
#include "private/storage/file_manager.h"

// Public
PageManager *page_manager_new() {
    PageManager *pm = malloc(sizeof(PageManager));
    ASSERT_NOT_NULL(pm, FAILED_TO_ALLOCATE_MEMORY)
    pm->file_manager = file_manager_new();
    pm->pages = NULL;
    pm->pages_in_memory = 0;
    pm->current_free_page = NULL;
    return pm;
}

Result page_manager_init(PageManager *self, const char *filename, uint32_t page_size, int32_t file_signature) {
    ASSERT_ARG_NOT_NULL(self);

    //TODO: persist on disk page-manager's data in file-header.
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
    self->current_free_page = current_free_page;

    return OK;
}

void page_manager_destroy(PageManager *self) {
    ASSERT_ARG_NOT_NULL(self);

    file_manager_destroy(self->file_manager);
    // TODO: remove all pages from memory
    for (size_t i = 0; i < self->pages_in_memory; i++) {
        Page *next_page = self->pages->page_metadata.next_page;
        page_destroy(self->pages);
        self->pages = next_page;
    }
    // TODO: check that we don't do double-free
    // free(self->current_free_page);
}

void page_manager_after_page_read(PageManager *self, Page* page) {
    // it's first page
    if (self->pages == NULL) {
        self->pages = page;
    } else {
        self->pages->page_metadata.next_page = page;
    }
    self->pages_in_memory++;
}

Result page_manager_get_first_page_or_create(PageManager *self, Page **result) {
    ASSERT_ARG_NOT_NULL(self);
    ASSERT_ARG_IS_NULL(*result);

    if (page_manager_get_pages_count(self) == 0) {
        return page_manager_page_new(self, result);
    }
    return page_manager_read_page(self, page_id(0), result);
}

/*
 * Allocates new page
 */
Result page_manager_page_new(PageManager *self, Page **page) {
    ASSERT_ARG_NOT_NULL(self);
    ASSERT_ARG_IS_NULL(*page);

    page_index_t next_id = next_page(page_manager_get_last_page_id(self));
    *page = page_new(next_id, page_manager_get_page_size(self));
    uint32_t old_pages_count = page_manager_get_pages_count(self);
    Result res = page_manager_set_pages_count(self, old_pages_count++);
    RETURN_IF_FAIL(res, "Failed increment pages count")
    // write to file
    uint32_t page_size = page_manager_get_page_size(self);
    Result page_write_res = file_manager_write(self->file_manager, (*page)->page_header.file_offset, page_size,
                                               page);
    // TODO: free page if fail
    RETURN_IF_FAIL(page_write_res, "Failed to write new page to file")

    page_manager_after_page_read(self, *page);

    return OK;
}

// TODO: why do we need this method?
void page_manager_page_destroy(PageManager *self, Page *page) {
    ASSERT_ARG_NOT_NULL(self);
    ASSERT_ARG_NOT_NULL(page);

    return page_destroy(page);
}

Result page_manager_flush_page(PageManager *self, Page *page) {
    ASSERT_ARG_NOT_NULL(self);
    ASSERT_ARG_NOT_NULL(page);

    size_t offset_in_file = page_manager_get_page_offset(self, page->page_header.page_id);
    size_t size = page_size(page);

    file_manager_write(self->file_manager, offset_in_file, size, page);
    return OK;
}

// Private
Result page_manager_get_page_from_ram(PageManager *self, page_index_t page_id, Page **result) {
    ASSERT_ARG_NOT_NULL(self);
    ASSERT_ARG_IS_NULL(result);

    // for each page in pages
    Page *current_page = self->pages;
    for (size_t i = 0; i < self->pages_in_memory; i++) {
        if (current_page->page_header.page_id.id == page_id.id) {
            *result = current_page;
            return OK;
        }
        current_page = current_page->page_metadata.next_page;
    }
    return ERROR("Page not found in ram");
}
