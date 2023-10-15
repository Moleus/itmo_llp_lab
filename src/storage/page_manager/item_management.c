#include "private/storage/page_manager.h"

//TODO: think if it's correct to include private header
#include "private/storage/page.h"
#include "private/storage/file_manager.h"

#define ITEM_METADATA_SIZE ((int32_t) sizeof(ItemMetadata))

Result persist_item_on_disk(PageManager *self, Page *page, ItemPayload *payload, ItemAddResult *add_result) {
    uint32_t metadata_offset = add_result->metadata_offset_in_page;
    uint32_t data_offset = add_result->metadata.data_offset;
    ItemMetadata metadata = add_result->metadata;

    size_t metadata_offset_in_file = convert_to_file_offset(self, page->page_header.page_id, metadata_offset);
    Result res = file_manager_write(self->file_manager, metadata_offset_in_file, ITEM_METADATA_SIZE,
                             (void *) &metadata);
    //TODO: think about operation rollback. we might need to remove added item from page if fail
    RETURN_IF_FAIL(res, "Failed to write item metadata to file")

    // persist data on disk
    size_t data_offset_in_file = convert_to_file_offset(self, page->page_header.page_id, data_offset);
    res = file_manager_write(self->file_manager, data_offset_in_file, (*payload).size, (*payload).data);
    RETURN_IF_FAIL(res, "Failed to write item data to file")

    // persist header on disk
    size_t header_offset_in_file = page_manager_get_page_offset(self,page->page_header.page_id);
    res = file_manager_write(self->file_manager, header_offset_in_file, sizeof(PageHeader), &page->page_header);
    RETURN_IF_FAIL(res, "Failed to write page header to file")

    return OK;
}

Result page_manager_put_item(PageManager *self, Page *page, ItemPayload payload, ItemAddResult *item_add_result) {
    ASSERT_ARG_NOT_NULL(self);
    ASSERT_ARG_NOT_NULL(page);
    ASSERT_ARG_NOT_NULL(item_add_result);

    // persist in memory
    Result res = page_add_item(page, payload, item_add_result);
    RETURN_IF_FAIL(res, "Failed to add item to page in memory")

    // Persist head of the item on disk. We return only info about the first part in itemAddResult
    persist_item_on_disk(self, page, &payload, item_add_result);

    ItemAddResult tmp_add_result = *item_add_result;
    // if we don't have enough space in page then we need to allocate new page and place left data there
    while (tmp_add_result.write_status.complete == false) {
        Page *free_page = NULL;
        res = page_manager_page_new(self, &free_page);
        ABORT_IF_FAIL(res, "Failed to allocate one more page for large payload")

        // !!! update current_free page. Forget about the old one.
        res = page_manager_set_current_free_page(self, free_page);
        ABORT_IF_FAIL(res, "Failed to update current free page");
        ItemPayload payload_to_write = {
                .data = payload.data + item_add_result->write_status.bytes_left,
                .size = item_add_result->write_status.bytes_left
        };
        res = page_add_item(free_page, payload_to_write, &tmp_add_result);
        ABORT_IF_FAIL(res, "Failed to add other large part of item to page in memory");
        tmp_add_result.metadata.continues_on_page = free_page->page_header.page_id;

        // persist second part
        res = persist_item_on_disk(self, page, &payload, &tmp_add_result);
        ABORT_IF_FAIL(res, "Failed to write part of large payload to file");

        // TODO: check
        item_add_result->metadata.continues_on_page = free_page->page_header.page_id;
    }
    // TODO: check while loop end condition
    return OK;
}

Result page_manager_delete_item(PageManager *self, Page *page, Item *item) {
    ASSERT_ARG_NOT_NULL(self);
    ASSERT_ARG_NOT_NULL(page);
    ASSERT_ARG_NOT_NULL(item);


    // persist in memory
    Result res = page_delete_item(page, item);
    RETURN_IF_FAIL(res, "Failed to delete item from page in memory")

    // we don't need to write deleted data. We only flush header
    size_t header_offset_in_file = page_manager_get_page_offset(self,page->page_header.page_id);
    res = file_manager_write(self->file_manager, header_offset_in_file, sizeof(PageHeader), &page->page_header);
    RETURN_IF_FAIL(res, "Failed to write page header to file")

    // TODO: implement defragmentation
    return OK;
}
