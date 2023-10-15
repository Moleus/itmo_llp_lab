#include "private/storage/page_manager.h"

//TODO: think if it's correct to include private header
#include "private/storage/page.h"
#include "private/storage/file_manager.h"

#define ITEM_METADATA_SIZE ((int32_t) sizeof(ItemMetadata))

// Private
// get page offset by id
size_t page_manager_get_page_offset(PageManager *self, page_index_t page_id) {
    ASSERT_ARG_NOT_NULL(self);

    return (size_t) page_id.id * page_manager_get_page_size(self);
}

page_index_t page_manager_get_last_page_id(PageManager *self) {
    ASSERT_ARG_NOT_NULL(self);

    int32_t pages_count = page_manager_get_pages_count(self);
    return page_id(pages_count - 1);
}

// Public
PageManager *page_manager_new() {
    PageManager *pm = malloc(sizeof(PageManager));
    ASSERT_NOT_NULL(pm, FAILED_TO_ALLOCATE_MEMORY);
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
    RETURN_IF_FAIL(res, "Failed to read file header");

    // this works both with empty file and file which contains data
    page_index_t free_page_id = page_id(self->file_manager->header.dynamic.current_free_page);
    Page *current_free_page = NULL;
    page_manager_read_page(self, free_page_id, &current_free_page);
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

/*
 * Allocates new page
 */
Result page_manager_page_new(PageManager *self, Page **page) {
    ASSERT_ARG_NOT_NULL(self);
    ASSERT_ARG_IS_NULL(*page);

    page_index_t next_id = next_page(page_manager_get_last_page_id(self));
    *page = page_new(next_id, page_manager_get_page_size(self));
    int32_t old_pages_count = page_manager_get_pages_count(self);
    Result res = page_manager_set_pages_count(self, old_pages_count++);
    RETURN_IF_FAIL(res, "Failed increment pages count");
    // write to file
    int32_t page_size = page_manager_get_page_size(self);
    Result page_write_res = file_manager_write(self->file_manager, (*page)->page_header.file_offset, page_size,
                                               page);
    // TODO: free page if fail
    RETURN_IF_FAIL(page_write_res, "Failed to write new page to file");

    self->pages->page_metadata.next_page = *page;
    self->pages_in_memory++;

    return OK;
}

// TODO: why do we need this method?
void page_manager_page_destroy(PageManager *self, Page *page) {
    ASSERT_ARG_NOT_NULL(self);
    ASSERT_ARG_NOT_NULL(page);

    return page_destroy(page);
}

/*
 * Page should exist in memory or on disk
 * If page doesn't exist in memory then it will be loaded from disk
 */
Result page_manager_read_page(PageManager *self, page_index_t id, Page **result_page) {
    ASSERT_ARG_NOT_NULL(self);
    ASSERT_ARG_IS_NULL(*result_page);

    int32_t pages_count = page_manager_get_pages_count(self);
    if (id.id >= pages_count) {
        ABORT_EXIT(INTERNAL_LIB_ERROR, "Page doesn't exist");
    }

    // check if page exists in ram
    Result res = page_manager_get_page_from_ram(self, id, result_page);
    if (res.status == RES_OK) {
        // page found in ram
        return OK;
    }

    // TODO: can we use page_manager_page_new instead?
    // no. Because here we get page by id. And new only allocates new page

    // pass offset to file_manger and get page
    size_t offset = page_manager_get_page_offset(self, id);
    // load from disk
    // allocate page here
    int32_t page_size = page_manager_get_page_size(self);
    Page *page = page_new(id, page_size);
    // read header
    res = file_manager_read(self->file_manager, offset, sizeof(PageHeader), page);
    RETURN_IF_FAIL(res, "Failed to read page header from file")
    // read payload
    res = file_manager_read(self->file_manager, offset + sizeof(PageHeader), page_get_payload_size(page_size),
                            page->page_payload.bytes);
    RETURN_IF_FAIL(res, "Failed to read page payload from file");

    // TODO: don't forget about this page in memory
    self->pages->page_metadata.next_page = page;
    self->pages_in_memory++;

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

static size_t convert_to_file_offset(PageManager *self, page_index_t page_id, size_t offset_in_page) {
    return page_manager_get_page_offset(self, page_id) + offset_in_page;
}

Result page_manager_put_item(PageManager *self, Page *page, ItemPayload payload, ItemAddResult *item_add_result) {
    ASSERT_ARG_NOT_NULL(self);
    ASSERT_ARG_NOT_NULL(page);

    // persist in memory
    Result res = page_add_item(page, payload, item_add_result);
    RETURN_IF_FAIL(res, "Failed to add item to page in memory");

    // if we don't have enough space in page then we need to allocate new page and place left data there
    if (!item_add_result->write_status.complete) {
        //TODO: we have to modify metadata to set continues_on_page
        ItemAddResult free_page_item_add_result;
        Page *free_page = NULL;
        res = page_manager_page_new(self, &free_page);
        // !!! update current_free page. Forget about the old one.
        page_manager_set_current_free_page(self, free_page);
        RETURN_IF_FAIL(res, "Failed to allocate one more page for large payload")
        ItemPayload payload_to_write = {
                .data = payload.data + item_add_result->write_status.bytes_left,
                .size = item_add_result->write_status.bytes_left
        };
        res = page_manager_put_item(self, free_page, payload_to_write, &free_page_item_add_result);
        ABORT_IF_FAIL(res, "Failed to write part of large payload to file")

        // TODO: !!! FUCKING IMPORTANT !!! test that we set next page correctly inside recursion
        item_add_result->metadata.continues_on_page = free_page->page_header.page_id;
    }

    ItemMetadata metadata = item_add_result->metadata;
    int32_t metadata_offset = item_add_result->metadata_offset_in_page;
    int32_t data_offset = item_add_result->metadata.data_offset;

    // persist metadata on disk
    size_t metadata_offset_in_file = convert_to_file_offset(self, page->page_header.page_id, metadata_offset);
    res = file_manager_write(self->file_manager, metadata_offset_in_file, ITEM_METADATA_SIZE,
                             (void *) &metadata);
    //TODO: think about operation rollback. we might need to remove added item from page if fail
    RETURN_IF_FAIL(res, "Failed to write item metadata to file");

    // persist data on disk
    size_t data_offset_in_file = convert_to_file_offset(self, page->page_header.page_id, data_offset);
    res = file_manager_write(self->file_manager, data_offset_in_file, payload.size, payload.data);
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

Page *page_manager_get_current_free_page(PageManager *self) {
    ASSERT_ARG_NOT_NULL(self);

    return self->current_free_page;
}

Result page_manager_set_current_free_page(PageManager *self, Page *page) {
    ASSERT_ARG_NOT_NULL(self);
    ASSERT_ARG_NOT_NULL(page);

    self->file_manager->header.dynamic.current_free_page = page->page_header.page_id.id;
    self->current_free_page = page;
    return file_manager_write_header(self->file_manager);
}

int32_t page_manager_get_page_size(PageManager *self) {
    ASSERT_ARG_NOT_NULL(self);

    return self->file_manager->header.constants.page_size;
}

int32_t page_manager_get_pages_count(PageManager *self) {
    ASSERT_ARG_NOT_NULL(self);

    return self->file_manager->header.dynamic.page_count;
}

Result page_manager_set_pages_count(PageManager *self, int32_t pages_count) {
    ASSERT_ARG_NOT_NULL(self);

    self->file_manager->header.dynamic.page_count = pages_count;
    return file_manager_write_header(self->file_manager);
}