#include "private/storage/page_manager.h"

//TODO: think if it's correct to include private header
#include "private/storage/page.h"

#define ITEM_METADATA_SIZE ((int32_t) sizeof(ItemMetadata))

// Private
// get page offset by id
size_t page_manager_get_page_offset(PageManager *self, page_index_t page_id) {
    ASSERT_ARG_NOT_NULL(self);

    return (size_t) page_id.id * self->page_size;
}

page_index_t page_manager_get_last_page_id(PageManager *self) {
    ASSERT_ARG_NOT_NULL(self);

    return page_id(self->pages_count - 1);
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

    page_index_t next_id = next_page(page_manager_get_last_page_id(self));
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

    return page_destroy(page);
}

/*
 * Page should exist in memory or on disk
 * If page doesn't exist in memory then it will be loaded from disk
 */
Result page_manager_read_page(PageManager *self, page_index_t id, Page *result_page) {
    ASSERT_ARG_NOT_NULL(self);
    ASSERT_ARG_IS_NULL(result_page);

    if (id.id >= self->pages_count) {
        return ERROR("Page doesn't exist");
    }

    // check if page exists in ram
    Result res = page_manager_get_page_from_ram(self, id, result_page);
    if (res.status == RES_OK) {
        // page found in ram
        return OK;
    }

    // pass offset to file_manger and get page
    size_t offset = page_manager_get_page_offset(self, id);

    // load from disk
    // allocate page here
    Page *page;
    res = page_new(id, self->page_size, &page);
    RETURN_IF_FAIL(res, "Failed to create page");

    // read header
    res = file_manager_read(self->file_manager, offset, sizeof(PageHeader), page);
    RETURN_IF_FAIL(res, "Failed to read page header from file")

    // read payload
    res = file_manager_read(self->file_manager, offset + sizeof(PageHeader), page_get_payload_size(self->page_size), page->page_payload.bytes);
    RETURN_IF_FAIL(res, "Failed to read page payload from file");

    self->pages_in_memory++;
    // TODO: don't forget about this page in memory
    self->pages->page_metadata.next_page = page;

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
//Result page_manager_get_free_page(PageManager *self, Page *page) {
//    ASSERT_ARG_NOT_NULL(self);
//    ASSERT_ARG_IS_NULL(page);
//
//    int32_t next_page_id = page_manager_get_next_page_id(self);
//    Result new_page_res = page_manager_page_new(self, page);
//    RETURN_IF_FAIL(new_page_res, "Failed to create new page");
//
//    return OK;
//}

// Private
Result page_manager_get_page_from_ram(PageManager *self, page_index_t page_id, Page *result) {
    ASSERT_ARG_NOT_NULL(self);
    ASSERT_ARG_IS_NULL(result);

    // for each page in pages
    Page* current_page = self->pages;
    for (size_t i = 0; i < self->pages_in_memory; i++) {
        if (current_page->page_header.page_id.id == page_id.id) {
            *result = *current_page;
            return OK;
        }
        current_page = current_page->page_metadata.next_page;
    }
    return ERROR("Page not found in ram");
}

static size_t convert_to_file_offset(PageManager *self, page_index_t page_id, size_t offset_in_page) {
    return page_manager_get_page_offset(self, page_id) + offset_in_page;
}

Result page_manager_put_item(PageManager *self, Page *page, Item *item) {
    ASSERT_ARG_NOT_NULL(self);
    ASSERT_ARG_NOT_NULL(page);
    ASSERT_ARG_NOT_NULL(item);

    // persist in memory
    ItemResult item_add_result;
    Result res = page_add_item(page, item, &item_add_result);
    RETURN_IF_FAIL(res, "Failed to add item to page in memory");

    ItemMetadata metadata = item_add_result.metadata;
    int32_t metadata_offset = item_add_result.metadata_offset_in_page;
    int32_t data_offset = item_add_result.metadata.data_offset;

    // persist metadata on disk
    size_t metadata_offset_in_file = convert_to_file_offset(self, page->page_header.page_id, metadata_offset);
    res = file_manager_write(self->file_manager, metadata_offset_in_file, ITEM_METADATA_SIZE,
                                    (void *) &metadata);
    //TODO: think about operation rollback. we might need to remove added item from page if fail
    RETURN_IF_FAIL(res, "Failed to write item metadata to file");

    // persist data on disk
    size_t data_offset_in_file = convert_to_file_offset(self, page->page_header.page_id, data_offset);
    res = file_manager_write(self->file_manager, data_offset_in_file, item->size, item->data);
    RETURN_IF_FAIL(res, "Failed to write item data to file");

    // persist header on disk
    size_t header_offset_in_file = page->page_header.file_offset;
    res = file_manager_write(self->file_manager, header_offset_in_file, sizeof(PageHeader), &page->page_header);
    RETURN_IF_FAIL(res, "Failed to write page header to file");

    return OK;
}

Result page_manager_delete_item(PageManager *self, Page *page, Item *item) {
    ASSERT_ARG_NOT_NULL(self);
    ASSERT_ARG_NOT_NULL(page);
    ASSERT_ARG_NOT_NULL(item);


    // persist in memory
    Result res = page_delete_item(page, item);
    RETURN_IF_FAIL(res, "Failed to delete item from page in memory");

    // we don't need to write deleted data. We only flush header
    size_t header_offset_in_file = page->page_header.file_offset;
    res = file_manager_write(self->file_manager, header_offset_in_file, sizeof(PageHeader), &page->page_header);
    RETURN_IF_FAIL(res, "Failed to write page header to file");

    // TODO: implement defragmentation
    return OK;
}

Result page_manager_get_item();

// helper which maps page offset into global file offsets
Result page_manager_get_page_offsets(PageManager *self, page_index_t page_id, size_t *start_offset, size_t *end_offset) {
    ASSERT_ARG_NOT_NULL(self);
    ASSERT_ARG_NOT_NULL(start_offset);
    ASSERT_ARG_NOT_NULL(end_offset);

    size_t page_offset = page_manager_get_page_offset(self, page_id);
    *start_offset = page_offset + page_get_data_offset();
    *end_offset = page_offset + self->page_size;
    return OK;
}