#include "private/storage/page_manager.h"

//TODO: think if it's correct to include private header
#include "private/storage/page.h"
#include "private/storage/file_manager.h"

// Private
// get page offset by id
size_t page_manager_get_page_offset(PageManager *self, page_index_t page_id) {
    ASSERT_ARG_NOT_NULL(self)

    return sizeof(FileHeader) + (size_t) page_id.id * page_manager_get_page_size(self);
}

page_index_t page_manager_get_last_page_id(PageManager *self) {
    ASSERT_ARG_NOT_NULL(self)

    int32_t pages_count = (int32_t) page_manager_get_pages_count(self);
    return page_id(pages_count - 1);
}

Page *page_manager_get_current_free_page(PageManager *self) {
    ASSERT_ARG_NOT_NULL(self)

    if (self->current_free_page == NULL) {
        page_index_t free_page_id = page_id(self->file_manager->header.dynamic.current_free_page);
        Result res = page_manager_read_page(self, free_page_id, &self->current_free_page);
        ABORT_IF_FAIL(res, "Failed to read current free page from disk");
    }
    LOG_DEBUG("Get current free page %d", self->current_free_page->page_header.page_id.id);
    return self->current_free_page;
}

Result page_manager_set_current_free_page(PageManager *self, Page *page) {
    ASSERT_ARG_NOT_NULL(self)
    ASSERT_ARG_NOT_NULL(page)

    self->file_manager->header.dynamic.current_free_page = page->page_header.page_id.id;
    self->current_free_page = page;
    return file_manager_write_header(self->file_manager);
}

uint32_t page_manager_get_page_size(PageManager *self) {
    ASSERT_ARG_NOT_NULL(self)

    return self->file_manager->header.constants.page_size;
}

uint32_t page_manager_get_pages_count(PageManager *self) {
    ASSERT_ARG_NOT_NULL(self)

    return self->file_manager->header.dynamic.page_count;
}

Result page_manager_set_pages_count(PageManager *self, uint32_t pages_count) {
    ASSERT_ARG_NOT_NULL(self)

    self->file_manager->header.dynamic.page_count = pages_count;
    return file_manager_write_header(self->file_manager);
}

size_t convert_to_file_offset(PageManager *self, page_index_t page_id, size_t offset_in_page) {
    return page_manager_get_page_offset(self, page_id) + offset_in_page;
}

