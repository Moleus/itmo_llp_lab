#include "private/storage/page_manager.h"

//TODO: think if it's correct to include private header
#include "private/storage/page.h"

#define ITEM_METADATA_SIZE sizeof(ItemMetadata)

// Private
// get page offset by id
size_t page_manager_get_page_offset(PageManager *self, size_t page_id) {
    ASSERT_ARG_NOT_NULL(self);

    return (size_t) page_id * self->page_size;
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
    page_manager_read_page(self, page_id, page);
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

    // TODO: remove all pages from memory
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
    Result new_page_res = page_new(next_id, self->page_size, page);
    RETURN_IF_FAIL(new_page_res, "Failed to create page");
    self->pages_count++;
    // TODO: add page to file
    // TODO: check file offset
    Result page_write_res = file_manager_write(self->file_manager, page->page_header.file_offset, self->page_size, page);
    RETURN_IF_FAIL(page_write_res, "Failed to write new page to file");

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

/*
 * Page should exist in memory or on disk
 * If page doesn't exist in memory then it will be loaded from disk
 */
Result page_manager_read_page(PageManager *self, size_t id, Page *page) {
    ASSERT_ARG_NOT_NULL(self);
    ASSERT_ARG_IS_NULL(page);

    if (id >= self->pages_count) {
        return ERROR("Page doesn't exist");
    }
    // check if memory exists in ram
    Result res = page_manager_get_page_from_ram(self, id, page);

    // pass offset to file_manger and get page
    size_t offset = page_manager_get_page_offset(self, id);
    size_t size = page_size(page);

    file_manager_read(self->file_manager, offset, size, page);
    return OK;
}

Result page_manager_flush_page(PageManager *self, Page *page) {
    ASSERT_ARG_NOT_NULL(self);
    ASSERT_ARG_NOT_NULL(page);

    size_t offset_in_file = page_manager_get_page_offset(self, page->page_header.page_id);
    size_t size = page_size(page);

    file_manager_write(self->file_manager, offset_in_file, size, page);
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

// Private
Result page_manager_get_page_from_ram(PageManager *self, size_t page_id, Page *result) {
    ASSERT_ARG_NOT_NULL(self);
    ASSERT_ARG_IS_NULL(result);

    // for each page in pages
    Page* current_page = self->pages;
    for (size_t i = 0; i < self->pages_count; i++) {
        if (current_page->page_header.page_id == page_id) {
            *result = *current_page;
            return OK;
        }
        current_page = current_page->page_metadata.next_page;
    }
    return ERROR("Page not found in ram");
}

static size_t convert_to_file_offset(PageManager *self, int32_t page_id, size_t offset_in_page) {
    return page_manager_get_page_offset(self, page_id) + offset_in_page;
}

Result page_manager_put_item(PageManager *self, Page *page, Item *item) {
    ASSERT_ARG_NOT_NULL(self);
    ASSERT_ARG_NOT_NULL(page);

    // check that we have enough size to put item metadata and item data into free-range
    size_t free_space_size = page->page_header.free_space_end_offset - page->page_header.free_space_start_offset;
    if (item->size > free_space_size) {
        return ERROR("Not enough space to put item");
    }

    // we place data in the reverse order staring from page end
    int32_t data_offset = page->page_header.free_space_end_offset - item->size;
    size_t item_id = page->page_header.items_count;
    ItemMetadata metadata = {
            .id = (int32_t) item_id,
            .offset = (int32_t) data_offset,
            .size = item->size
    };
    

    int32_t metadata_offset = page->page_header.free_space_start_offset;

    // convert to file offsets
    size_t metadata_offset_in_file = convert_to_file_offset(self, page->page_header.page_id, metadata_offset);
    Result res = file_manager_write(self->file_manager, metadata_offset_in_file, ITEM_METADATA_SIZE,
                                    (void *) &metadata);
    RETURN_IF_FAIL(res, "Failed to write item metadata to file");

    size_t data_offset_in_file = convert_to_file_offset(self, page->page_header.page_id, data_offset);
    res = file_manager_write(self->file_manager, data_offset_in_file, item->size, item->data);
    RETURN_IF_FAIL(res, "Failed to write item data to file");

    // after all operations succeeded we can update page metadata
    page->page_header.free_space_start_offset = metadata_offset + ITEM_METADATA_SIZE;
    page->page_header.free_space_end_offset = data_offset;
    page->page_header.items_count++;
}

// helper which maps page offset into global file offsets
Result page_manager_get_page_offsets(PageManager *self, size_t page_id, size_t *start_offset, size_t *end_offset) {
    ASSERT_ARG_NOT_NULL(self);
    ASSERT_ARG_NOT_NULL(start_offset);
    ASSERT_ARG_NOT_NULL(end_offset);

    size_t page_offset = page_manager_get_page_offset(self, page_id);
    *start_offset = page_offset + page_get_data_offset();
    *end_offset = page_offset + page_size(page);
    return OK;
}