#include <assert.h>
#include "private/storage/page_manager.h"

//TODO: think if it's correct to include private header
#include "private/storage/page.h"
#include "private/storage/file_manager.h"

#define ITEM_METADATA_SIZE ((int32_t) sizeof(ItemMetadata))

Result write_page_on_disk(PageManager *self, Page *page) {
    // persist page on disk
    size_t page_offset_in_file = page_manager_get_page_offset(self, page->page_header.page_id);
    uint32_t page_size = page_manager_get_page_size(self);
    Result res = file_manager_write(self->file_manager, page_offset_in_file, page_size, page);
    RETURN_IF_FAIL(res, "Failed to write page header to file")
    LOG_DEBUG("Saved page %d on disk", page->page_header.page_id.id);

    return OK;
}

Result page_manager_add_part_of_item(PageManager *self, Page *page, ItemPayload payload, page_index_t continues_on_page,
                                     ItemAddResult *item_add_result) {
    ASSERT_ARG_NOT_NULL(self)
    ASSERT_ARG_NOT_NULL(page)
    ASSERT_ARG_NOT_NULL(item_add_result)

    Result res = page_add_item(page, payload, continues_on_page, item_add_result);
    RETURN_IF_FAIL(res, "Failed to add item to page in memory")

    assert(item_add_result->metadata_offset_in_page >= sizeof(PageHeader));
    LOG_DEBUG("Add item %d to page %d. iSize: %d. Status: %b. Offset in Page %d. Total items: %d",
              item_add_result->metadata.item_id.id,
              page->page_header.page_id.id, item_add_result->metadata.size, item_add_result->write_status.complete,
              item_add_result->metadata_offset_in_page, page->page_header.items_count);
    // Persist head of the item on disk. We return only info about the first part in itemAddResult
    return write_page_on_disk(self, page);
}

Result page_manager_get_new_free_page(PageManager *self, Page **free_page) {
    Result res = page_manager_page_new(self, free_page);
    ABORT_IF_FAIL(res, "Failed to allocate one more page for large payload")
    // !!! update current_free page. Forget about the old one.
    return page_manager_set_current_free_page(self, *free_page);
}

Result page_manager_put_item(PageManager *self, Page *page, ItemPayload payload, ItemAddResult *item_add_result) {
    ASSERT_ARG_NOT_NULL(self)
    ASSERT_ARG_NOT_NULL(page)
    ASSERT_ARG_NOT_NULL(item_add_result)
    ASSERT_ARG_NOT_NULL(payload.data);
    assert(payload.size > 0);

    page_index_t continue_on_page = NULL_PAGE_INDEX;
    uint32_t bytes_written = 0;
    ItemAddResult *tmp_add_result = item_add_result;
    ItemAddResult tmp_add_result_storage;
    Page *current_page = page;

    // if we don't have enough space in page then we need to allocate new page and place left data there
    while (bytes_written < payload.size) {
        // If it is not the first iteration then the current_page should have been allocated
        assert(current_page != NULL);
        Page *free_page = NULL;
        uint32_t payload_size = payload.size - bytes_written;
        if (page_can_fit_payload(current_page, payload_size) == false) {
            // early allocate next page
            Result res = page_manager_get_new_free_page(self, &free_page);
            ABORT_IF_FAIL(res, "Failed to allocate one more page for large payload")
            continue_on_page = free_page->page_header.page_id;
            payload_size = page_get_payload_available_space(current_page);
        }
        ItemPayload payload_to_write = {
                // TODO: can add pointers?
                .data = payload.data + bytes_written,
                .size = payload_size
        };
        Result res = page_manager_add_part_of_item(self, current_page, payload_to_write, continue_on_page,
                                                   tmp_add_result);
        ABORT_IF_FAIL(res, "Failed to add item to page in memory")

        assert(tmp_add_result->write_status.bytes_written == payload_to_write.size);
        bytes_written += tmp_add_result->write_status.bytes_written;
        tmp_add_result = &tmp_add_result_storage;
        assert(tmp_add_result != item_add_result);
        current_page = free_page;
    }
    item_add_result->write_status = tmp_add_result->write_status;
    item_add_result->write_status.bytes_written = bytes_written;

    assert(bytes_written == payload.size);
    assert(item_add_result->write_status.complete == true);
    return OK;
}

Result page_manager_delete_item(PageManager *self, Page *page, Item *item) {
    ASSERT_ARG_NOT_NULL(self)
    ASSERT_ARG_NOT_NULL(page)
    ASSERT_ARG_NOT_NULL(item)

    Page *current_page = page;
    page_index_t current_page_idx = page_get_id(current_page);
    Item *item_to_delete = item;

    while (current_page_idx.id != NULL_PAGE_INDEX.id) {
        if (current_page_idx.id != page_get_id(page).id) {
            // is next page
            Result res = page_manager_read_page(self, current_page_idx, &current_page);
            ABORT_IF_FAIL(res, "Failed to get page from disk")
            // continuation of item should always be the first item in page;
            item_to_delete->index_in_page.id = 0;
        }
        // persist in memory
        Result res = page_delete_item(current_page, item_to_delete);
        ABORT_IF_FAIL(res, "Failed to delete item from page in memory")
        // persist on disk
        res = write_page_on_disk(self, current_page);
        ABORT_IF_FAIL(res, "Failed to write page on disk")

        LOG_INFO("Page %d. Item %d deleted. Items on page: %d", current_page->page_header.page_id.id, item->index_in_page.id,
                 item->index_in_page.id, page->page_header.items_count);
        // continue
        current_page_idx = page_get_item_continuation(current_page, item);
    }
    return OK;
}
