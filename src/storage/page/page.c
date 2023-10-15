#include <assert.h>
#include "private/storage/page.h"

/*
 * Represents only in-memory page data
 * It doesn't have access to disk
 * Allocates new page. Assigns id
 */
Page *page_new(page_index_t page_id, uint32_t page_size) {
    // TODO: rethink payload allocation
    Page *result = malloc(page_size);
    ASSERT_NOT_NULL(result, FAILED_TO_ALLOCATE_MEMORY)

    LOG_DEBUG("Allocated page with id %d, size: %d", page_id.id, page_size);

    PageHeader header = {
            .page_id = page_id,
            .items_count = 0,
            .next_item_id = 0,
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
}

static ItemMetadata *get_metadata(const Page *self, item_index_t item_id) {
    return (ItemMetadata *) (self->page_payload + sizeof(ItemMetadata) * item_id.id);
}

static uint8_t *get_item_data_addr(const Page *self, uint32_t data_offset) {
    return ((uint8_t *) self) + data_offset;
}

static Item create_item(const Page *self, ItemPayload payload) {
    return (Item) {.is_deleted = false, .index_in_page = self->page_header.next_item_id, .payload = payload};
}

// TODO: change public signature so user won't know anything about id and use just pointer
// maybe we don't even need this one
Result page_get_item(Page *self, item_index_t item_id, Item *item) {
    ASSERT_ARG_NOT_NULL(self)
    ASSERT_ARG_NOT_NULL(item)

    LOG_DEBUG("Getting item with id %d", item_id.id);
    if (item_id.id >= self->page_header.next_item_id.id) {
        return ERROR("Item id is out of range");
    }


    //TODO: привести все к формату, где создание item и укладываение в память будет в выделенной функции
    // продумать, что мы будем делать, когда страница выгрузится из памяти?
    // как возможное решение - копировать результат на уровне клиента.

    // TODO: check this address magic in tests
    ItemMetadata *metadata = get_metadata(self, item_id);
    assert(metadata->is_deleted == false);
    assert(self->page_header.next_item_id.id > item_id.id);

    // TODO: check that item pointer is assigned correctly
    *item = create_item(self, (ItemPayload) {.data = get_item_data_addr(self, metadata->data_offset), .size = metadata->size});

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
Result page_add_item_update_header(Page *self, ItemPayload payload, ItemAddResult *item_add_result) {
    // we place data in the reverse order staring from page end
    uint32_t data_offset = self->page_header.free_space_end_offset - payload.size;
    item_index_t item_id = self->page_header.next_item_id;
    ItemMetadata metadata = {
            .item_id = item_id,
            .data_offset = data_offset,
            .size = payload.size,
            .continues_on_page = NULL_PAGE_INDEX
    };

    *item_add_result = (ItemAddResult) {
            .metadata_offset_in_page = self->page_header.free_space_start_offset,
            .metadata = metadata,
            .write_status = (ItemWriteStatus) {.complete = true, .bytes_written = payload.size}
    };
    self->page_header.free_space_start_offset =
            item_add_result->metadata_offset_in_page + (uint32_t) sizeof(ItemMetadata);
    self->page_header.free_space_end_offset = data_offset;
    self->page_header.next_item_id.id++;
    self->page_header.items_count++;

    LOG_DEBUG("Page header update, offsets: %d/%d, item id: %d, payload size: %d, total items: %d, next_item_id: %d",
              self->page_header.free_space_start_offset, self->page_header.free_space_end_offset, item_id.id,
              payload.size, self->page_header.items_count, self->page_header.next_item_id.id);

    page_put_item_metadata_in_page(self, metadata);
    page_put_item_data_in_page(self, payload, data_offset);

    return OK;
}

// public
Result page_add_item(Page *self, ItemPayload payload, ItemAddResult *item_add_result) {
    ASSERT_ARG_NOT_NULL(self)
    ASSERT_ARG_NOT_NULL(item_add_result)

    if (payload.size > page_get_payload_available_space(self)) {
        LOG_WARN("Add failed. Page: %d, Available space: %d, payload size: %d", self->page_header.page_id.id,
                 page_get_payload_available_space(self), payload.size);
        ABORT_EXIT(INTERNAL_LIB_ERROR, "Can't add item to this page. Not enough space for metadata")
    }

    size_t free_space_size = page_get_free_space_left(self);
    if (payload.size > free_space_size) {
        LOG_INFO("Large payload. Page: %d, Available space: %d, payload size: %d", self->page_header.page_id.id,
                 page_get_payload_available_space(self), payload.size);
        // we can only write part of the payload. Split it into two parts
        ItemPayload partial_payload_head = payload;
        partial_payload_head.size = page_get_payload_available_space(self);
        page_add_item_update_header(self, partial_payload_head, item_add_result);
        item_add_result->write_status.complete = false;
        item_add_result->write_status.bytes_written = partial_payload_head.size;
        return OK;
    }
    page_add_item_update_header(self, payload, item_add_result);

    return OK;
}

Result page_delete_item(Page *self, Item *item) {
    ASSERT_ARG_NOT_NULL(self)
    ASSERT_ARG_NOT_NULL(item)
    assert(!item->is_deleted);
    assert(item->index_in_page.id < self->page_header.next_item_id.id);

    LOG_DEBUG("Page: %d. Deleting item with id %d", self->page_header.page_id.id, item->index_in_page.id);
    ItemMetadata *metadata = get_metadata(self, item->index_in_page);

    free(item->payload.data);
    item->is_deleted = true;
    self->page_header.items_count--;
    // if this was the last item in page then we can just move free space start offset
    //TODO: test this and check data consistency. Theoretically, we don't care about payload, but headers.
    // also, we should do defragmentation once in a time
    if (item->index_in_page.id == self->page_header.next_item_id.id - 1) {
        LOG_DEBUG("Page %d. Deleted last item %d", self->page_header.page_id.id, item->index_in_page.id);
        // increase free space by removing deleted data
        self->page_header.free_space_start_offset -= sizeof(ItemMetadata);
        self->page_header.free_space_end_offset += metadata->size;

        // decrement next_item_id until we find not deleted item
        // TODO: write updated page_header on disk
        int32_t last_item_index = self->page_header.next_item_id.id--;
        while (self->page_header.next_item_id.id > 0 && get_metadata(self, item_id(last_item_index)).is_deleted) {
            // TODO: does it change actual data in memory?
            last_item_index = self->page_header.next_item_id.id--;
        }
        LOG_DEBUG("Page %d. Updated next_item_id to %d after deletion", self->page_header.page_id.id,
                  self->page_header.next_item_id.id);
        // TODO: check if we can assign null item?
        //        *item = NULL_ITEM;
    }

    assert(item->is_deleted);
    return OK;
}