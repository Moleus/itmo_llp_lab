#include <assert.h>
#include "private/storage/page.h"

/*
 * Represents only in-memory page data
 * It doesn't have access to disk
 * Allocates new page. Assigns id
 */
Page *page_new(page_index_t page_id, uint32_t page_size) {
    Page *result = calloc(1, page_size);
    ASSERT_NOT_NULL(result, FAILED_TO_ALLOCATE_MEMORY)

    LOG_DEBUG("Allocated page with id %d, size: %d", page_id.id, page_size);

    PageHeader header = {
            .page_id = page_id,
            .items_count = 0,
            .next_item_id = {0},
            .free_space_start_offset = HEADER_SIZE,
            .free_space_end_offset = page_size,
            .page_size = page_size,
    };

    result->page_header = header;
    return result;
}

void page_destroy(Page *self) {
    ASSERT_ARG_NOT_NULL(self)

    LOG_DEBUG("Freeing page with id %d", self->page_header.page_id.id);
    free(self);
    self = NULL;
}

Result page_get_item(Page *self, item_index_t item_id, Item *item) {
    ASSERT_ARG_NOT_NULL(self)
    ASSERT_ARG_NOT_NULL(item)

    LOG_DEBUG("Getting item with id %d", item_id.item_id);
    if (item_id.item_id >= self->page_header.next_item_id.item_id) {
        LOG_ERR("Item id %d is out of range. Max id: %d", item_id.item_id, self->page_header.next_item_id.item_id - 1);
        return ERROR("Item id is out of range");
    }


    ItemMetadata *metadata = get_metadata(self, item_id);
    assert(metadata->is_deleted == false);
    assert(self->page_header.next_item_id.item_id > item_id.item_id);

    *item = create_item(
            (ItemPayload) {.size = metadata->size, .data = get_item_data_addr(self, metadata->data_offset)},
            item_id);

    LOG_DEBUG("Created item with id %d (or %d), size: %d, offset: %d", item_id.item_id, item->id.item_id, item->payload.size, metadata->data_offset);
    return OK;
}

void page_put_item_metadata_in_page(Page *self, ItemMetadata source) {
    ASSERT_ARG_NOT_NULL(self)

    ItemMetadata *dest = get_metadata(self, source.item_id);
    memcpy(dest, &source, sizeof(ItemMetadata));
}

void page_put_item_data_in_page(Page *self, ItemPayload payload, uint32_t data_offset) {
    ASSERT_ARG_NOT_NULL(self)

    uint8_t *addr = get_item_data_addr(self, data_offset);
    memcpy(addr, payload.data, payload.size);
}

// private
Result page_add_item_update_header(Page *self, ItemPayload payload, page_index_t continues_on_page,
                                   ItemAddResult *item_add_result) {
    // we place data in the reverse order staring from page end
    uint32_t data_offset = self->page_header.free_space_end_offset - payload.size;
    item_index_t item_id = self->page_header.next_item_id;
    item_id.page_id = self->page_header.page_id;
    ItemMetadata metadata = {
            .item_id = item_id,
            .data_offset = data_offset,
            .size = payload.size,
            .is_deleted = false,
            .continues_on_page = continues_on_page
    };

    bool is_complete = continues_on_page.id == NULL_PAGE_INDEX.id;
    *item_add_result = (ItemAddResult) {
            .metadata_offset_in_page = self->page_header.free_space_start_offset,
            .metadata = metadata,
            .write_status = (ItemWriteStatus) {.complete = is_complete, .bytes_written = payload.size}
    };
    self->page_header.free_space_start_offset =
            item_add_result->metadata_offset_in_page + (uint32_t) sizeof(ItemMetadata);
    self->page_header.free_space_end_offset = data_offset;
    self->page_header.next_item_id.item_id++;
    self->page_header.next_item_id.page_id = self->page_header.page_id;
    self->page_header.items_count++;

    LOG_DEBUG("Page header update, offsets: %d/%d, item id: %d, payload size: %d, total items: %d, next_item_id: %d",
              self->page_header.free_space_start_offset, self->page_header.free_space_end_offset, item_id.item_id,
              payload.size, self->page_header.items_count, self->page_header.next_item_id.item_id);

    page_put_item_metadata_in_page(self, metadata);
    page_put_item_data_in_page(self, payload, data_offset);

    return OK;
}


// public
/*
 * Payload should fit into page
 */
Result page_add_item(Page *self, ItemPayload payload, page_index_t continues_on_page, ItemAddResult *item_add_result) {
    ASSERT_ARG_NOT_NULL(self)
    ASSERT_ARG_NOT_NULL(item_add_result)

    if ((int32_t) payload.size > page_get_payload_available_space(self)) {
        LOG_ERR("Add failed. Page: %d, Available space: %d, payload size: %d", self->page_header.page_id.id,
                page_get_payload_available_space(self), payload.size);
        ABORT_EXIT(INTERNAL_LIB_ERROR, "Can't add item to this page. Not enough space for metadata")
    }

    page_add_item_update_header(self, payload, continues_on_page, item_add_result);

    return OK;
}

Result page_delete_item(Page *self, Item *item) {
    ASSERT_ARG_NOT_NULL(self)
    ASSERT_ARG_NOT_NULL(item)
    assert(!item->is_deleted);
    assert(item->id.item_id < self->page_header.next_item_id.item_id);
    assert(self->page_header.items_count > 0);

    LOG_DEBUG("Page: %d. Deleting item with id %d", self->page_header.page_id.id, item->id.item_id);
    ItemMetadata *metadata = get_metadata(self, item->id);

    item->is_deleted = true;
    metadata->is_deleted = true;
    self->page_header.items_count--;
    // if this was the last item in page then we can just move free space start offset
    if (item->id.item_id == self->page_header.next_item_id.item_id - 1) {
        LOG_DEBUG("Delete item - Page %d. Deleted last item %d", self->page_header.page_id.id, item->id.item_id);
        // increase free space by removing deleted data
        self->page_header.free_space_start_offset -= sizeof(ItemMetadata);
        self->page_header.free_space_end_offset += metadata->size;

        // decrement next_item_id until we find not deleted item

        self->page_header.next_item_id.item_id--;

        // Не дефрагментируем, т.к в больших айтемах придется менять индексы
//        int32_t last_item_index = self->page_header.next_item_id.id--;
//        while (self->page_header.next_item_id.id > 0 && get_metadata(self, item_id(last_item_index))->is_deleted) {
//            last_item_index = self->page_header.next_item_id.id--;
//        }
        LOG_DEBUG("Delete item - Page %d. Updated next_item_id to %d after deletion", self->page_header.page_id.id,
                  self->page_header.next_item_id.item_id);
    }

    assert(item->is_deleted);
    return OK;
}
