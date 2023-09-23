#include "storage/page_manager.h"

// Private
// get page offset by id
size_t page_manager_get_page_offset(PageManager *self, size_t page_id) {
    ASSERT_ARG_NOT_NULL(self);

    return (size_t) page_id * page_size();
}
int32_t page_manager_get_next_page_id(PageManager *self) {
    ASSERT_ARG_NOT_NULL(self);

    return (int32_t) self->pages_count;
}

int32_t page_manager_get_last_page_id(PageManager *self) {
    ASSERT_ARG_NOT_NULL(self);

    return (int32_t) self->pages_count - 1;
}

Result page_manager_set_next_page_id(PageManager *self, int32_t page_id) {
    ASSERT_ARG_NOT_NULL(self);

    Page *page;
    page_manager_get_page_by_id(self, page_id, page);
}

// Public
Result page_manager_new(PageManager *self, FileManager *file_manager) {
    ASSERT_ARG_NOT_NULL(self);

    // TODO: read pages count from file manager
    self->pages_count = 0;
    self->file_manager = file_manager;
    return OK;
}

Result page_manager_destroy(PageManager *self) {
    ASSERT_ARG_NOT_NULL(self);

    // TODO: remove all pages
    free(self->pages);
    return OK;
}

/*
 * Allocates new page
 */
Result page_manager_page_new(PageManager *self, Page *page) {
    ASSERT_ARG_NOT_NULL(self);
    ASSERT_ARG_IS_NULL(page);

    int32_t next_id = page_manager_get_last_page_id(self) + 1;
    Result new_page_res = page_new(next_id, page);
    RETURN_IF_FAIL(new_page_res, "Failed to create page");
    self->pages_count++;
    // TODO: add page to file

    return OK;
}

// TODO: why do we need this method?
Result page_manager_page_destroy(PageManager *self, Page *page) {
    ASSERT_ARG_NOT_NULL(self);
    ASSERT_ARG_NOT_NULL(page);

    Result destroy_res = page_destroy(page);
    RETURN_IF_FAIL(destroy_res, "Failed to destroy page");

    return OK;
}

Result page_manager_write_value(PageManager *self, int32_t page_num, Value value) {
    ASSERT_ARG_NOT_NULL(self);

    Page page;
    Result get_page_res = page_manager_get_page_by_id(self, page_num, &page);
    RETURN_IF_FAIL(get_page_res, "Failed to get page");

    Result write_res = page_write(&page, &value);
    RETURN_IF_FAIL(write_res, "Failed to write value to page");

    return OK;
}

/*
 * Page should be allocated
 */
Result page_manager_get_page_by_id(PageManager *self, size_t id, Page *page) {
    ASSERT_ARG_NOT_NULL(self);
    ASSERT_ARG_NOT_NULL(page);

    if (id >= self->pages_count) {
        return ERROR("Page doesn't exist");
    }

    // pass offset to file_manger and get page
    size_t offset = page_manager_get_page_offset(self, id);
    size_t size = page_size();

    file_manager_read(self->file_manager, offset, size, page);
    return OK;
}

// TODO: it's not page_manager responsibility to read data from page
Result page_manger_read(PageManager *self, size_t page_id, size_t item_id, Value *data) {
    ASSERT_ARG_NOT_NULL(self);
    ASSERT_ARG_NOT_NULL(data);

    Page page;
    Result get_page_res = page_manager_get_page_by_id(self, page_id, &page);
    RETURN_IF_FAIL(get_page_res, "Failed to get page");

    PageItem item;
    Result read_res = page_read(&page, item_id, &item);
    RETURN_IF_FAIL(read_res, "Failed to read data from page");
    *data = item.data;

    return OK;
}

/*
 * Allocates new page.
 */
Result page_manager_get_free_page(PageManager *self, Page *page) {
    ASSERT_ARG_NOT_NULL(self);
    ASSERT_ARG_IS_NULL(page);

    int32_t next_page_id = page_manager_get_next_page_id(self);
    Result new_page_res = page_manager_page_new(self, page);
    RETURN_IF_FAIL(new_page_res, "Failed to create new page");

    return OK;
}