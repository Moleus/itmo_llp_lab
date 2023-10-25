#include <assert.h>
#include "private/storage/page_manager.h"

#include "private/storage/page.h"
#include "private/storage/file_manager.h"

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
    LOG_DEBUG("Add item %d to page %d. iSize: %d. Status: %d. Offset in Page %d. Total items: %d",
              item_add_result->metadata.item_id.item_id,
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

    uint32_t bytes_written = 0;
    ItemAddResult tmp_add_result;
    Page *current_page = page;

    LOG_INFO("Put item to page %d. Payload size: %d", page->page_header.page_id.id, payload.size);

    // if we don't have enough space in page then we need to allocate new page and place left data there
    while (bytes_written < payload.size) {
        // If it is not the first iteration then the current_page should have been allocated
        assert(current_page != NULL);
        page_index_t continue_on_page = NULL_PAGE_INDEX;
        Page *free_page = NULL;
        int32_t payload_size = (int32_t) payload.size - (int32_t) bytes_written;
        if (page_can_fit_any_payload(current_page) == false) {
            // can't place even part of a payload
            Result res = page_manager_get_new_free_page(self, &free_page);
            current_page = free_page;
            ABORT_IF_FAIL(res, "Failed to allocate one more page for payload that can't suite page")
        } else if (page_can_fit_payload(current_page, payload_size) == false) {
            // early allocate next page
            Result res = page_manager_get_new_free_page(self, &free_page);
            ABORT_IF_FAIL(res, "Failed to allocate one more page for large payload")
            continue_on_page = free_page->page_header.page_id;
            payload_size = min(page_get_payload_available_space(current_page), payload_size);
        }
        ItemPayload payload_to_write = {
                .data = (void *) ((uint8_t *) payload.data + bytes_written),
                .size = payload_size
        };
        Result res = page_manager_add_part_of_item(self, current_page, payload_to_write, continue_on_page,
                                                   &tmp_add_result);
        ABORT_IF_FAIL(res, "Failed to add item to page in memory")

        if (page == current_page) {
            // first page. Save result
            *item_add_result = tmp_add_result;
        }
        assert(tmp_add_result.write_status.bytes_written == payload_to_write.size);
        bytes_written += tmp_add_result.write_status.bytes_written;
        current_page = free_page;
    }
    item_add_result->write_status = tmp_add_result.write_status; // final result must be complete
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
            current_page = NULL;
            Result res = page_manager_read_page(self, current_page_idx, &current_page);
            ABORT_IF_FAIL(res, "Delete item - Failed to get page from disk")
            // continuation of item should always be the first item in page;
            uint32_t payload_size;
            item_index_t _item_id = item_id(current_page->page_header.page_id, 0);
            page_manager_calculate_large_item_size(self, current_page, _item_id, &payload_size);
            uint8_t buffer[payload_size];
            LOG_DEBUG("Reading large item from page %d. Size: %d", current_page->page_header.page_id.id, payload_size);
            res = page_manager_get_item(self, current_page, _item_id, buffer, item_to_delete);
            ABORT_IF_FAIL(res, "Delete item - Failed to read item from page in memory")
            assert(item_to_delete->payload.size == payload_size);
        }
        // persist in memory
        Result res = page_delete_item(current_page, item_to_delete);
        ABORT_IF_FAIL(res, "Failed to delete item from page in memory")
        // persist on disk
        res = write_page_on_disk(self, current_page);
        ABORT_IF_FAIL(res, "Failed to write page on disk")

        LOG_INFO("Delete item - Page %d. Item %d deleted. Items on page: %d", current_page->page_header.page_id.id,
                 item_to_delete->id.item_id, page->page_header.items_count);
        // continue
        current_page_idx = page_get_item_continuation(current_page, item_to_delete);
    }
    assert(item_to_delete->is_deleted == true);
    return OK;
}

Result page_manager_calculate_large_item_size(PageManager *self, Page *page, item_index_t item_id, uint32_t *result) {
    ASSERT_ARG_NOT_NULL(self)
    ASSERT_ARG_NOT_NULL(page)
    ASSERT_ARG_NOT_NULL(result)

    Page *current_page = page;
    page_index_t current_page_idx = page_get_id(current_page);
    Item tmp_read_item = {0};
    tmp_read_item.id = item_id;
    *result = 0;

    LOG_DEBUG("Calculate large item size. Page: %d. Item: %d", page->page_header.page_id.id, item_id.item_id);

    while (current_page_idx.id != NULL_PAGE_INDEX.id) {
        if (current_page_idx.id != page_get_id(page).id) {
            // is next page
            current_page = NULL;
            Result res = page_manager_read_page(self, current_page_idx, &current_page);
            ABORT_IF_FAIL(res, "Failed to get page from disk")
            // continuation of item should always be the first item in page;
            tmp_read_item.id.item_id = 0;
            tmp_read_item.id.page_id = current_page->page_header.page_id;
        }
        // persist in memory
        Result res = page_get_item(current_page, tmp_read_item.id, &tmp_read_item);
        ABORT_IF_FAIL(res, "Failed to read item from page in memory")

        *result = *result + tmp_read_item.payload.size;

        // continue
        current_page_idx = page_get_item_continuation(current_page, &tmp_read_item);
    }
    return OK;
}

Result
page_manager_get_item(PageManager *self, Page *page, item_index_t item_id, uint8_t *payload_buffer, Item *result) {
    ASSERT_ARG_NOT_NULL(self)
    ASSERT_ARG_NOT_NULL(page)
    ASSERT_ARG_NOT_NULL(payload_buffer)
    ASSERT_ARG_NOT_NULL(result)

    Page *current_page = page;
    page_index_t current_page_idx = page_get_id(current_page);
    uint32_t item_cum_size = 0;
    Item *tmp_read_item = result;
    tmp_read_item->id = item_id;

    LOG_DEBUG("Get item. Page: %d. Item: %d", page->page_header.page_id.id, item_id.item_id);

    int double_loop_guard = 0;
    while (current_page_idx.id != NULL_PAGE_INDEX.id) {
        double_loop_guard++;
        if (current_page_idx.id != page_get_id(page).id) {
            double_loop_guard--;
            LOG_DEBUG("Item continues on page %d", current_page_idx.id);
            // is next page
            current_page = NULL;
            Result res = page_manager_read_page(self, current_page_idx, &current_page);
            ABORT_IF_FAIL(res, "Failed to get page from disk")
            // continuation of item should always be the first item in page;
            tmp_read_item->id.item_id = 0;
            tmp_read_item->id.page_id = current_page->page_header.page_id;
        }
        assert(double_loop_guard <= 1);
        // persist in memory
        Result res = page_get_item(current_page, tmp_read_item->id, tmp_read_item);
        ABORT_IF_FAIL(res, "Failed to read item from page in memory")
        uint32_t tmp_payload_size = tmp_read_item->payload.size;
        void *data = tmp_read_item->payload.data;
        LOG_DEBUG("Copying Size: %d. tmp_item_addr %p. Item Id %d. Cum size: %d",
                  tmp_payload_size, data, tmp_read_item->id.item_id,
                  item_cum_size);

        memcpy((void *) (payload_buffer + item_cum_size), data, tmp_payload_size);

        item_cum_size = item_cum_size + tmp_payload_size;
        LOG_DEBUG("Item ID after copy: %d, item_payload_size: %d", tmp_read_item->id.item_id,
                  tmp_read_item->payload.size);
        // continue
        current_page_idx = page_get_item_continuation(current_page, tmp_read_item);
    }
    result->id = item_id;
    result->payload.data = payload_buffer;
    result->payload.size = item_cum_size;
    LOG_DEBUG("Get item finished. Page: %d. Item: %d. Size: %d", page->page_header.page_id.id, item_id.item_id,
              item_cum_size);
    return OK;
}
