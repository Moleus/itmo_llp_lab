#include "private/storage/page.h"

#define HEADER_SIZE sizeof(PageHeader)

int32_t page_get_payload_size(int32_t page_size) {
    return page_size - (int32_t) HEADER_SIZE;
}

/*
 * Represents only in-memory page data
 * It doesn't have access to disk
 * Allocates new page. Assigns id
 */
Result page_new(page_index_t page_id, int32_t page_size, Page **result) {
    ASSERT_ARG_IS_NULL(result);

    // TODO: rethink payload allocation
    u_int8_t *payload = malloc(page_get_payload_size(page_size));
    RETURN_IF_NULL(payload, "Failed to allocate page payload");
    result = malloc(sizeof(Page));
    RETURN_IF_NULL(result, "Failed to allocate page")

    PageHeader header = {
            .page_id = page_id,
            .file_offset = (int32_t) page_id.id * page_size,
            .free_space_start_offset = HEADER_SIZE,
            .free_space_end_offset = page_size
    };

    (*result)->page_header = header;
    (*result)->page_payload.bytes = payload;
    return OK;
}

Result page_destroy(Page *self) {
    ASSERT_ARG_NOT_NULL(self);
    ASSERT_ARG_NOT_NULL(self->page_payload.bytes)

    free(self->page_payload.bytes);
    free(self);
    return OK;
}

int32_t page_get_data_offset() {
    return sizeof(PageHeader);
}

static ItemMetadata get_metadata(const Page *self, item_index_t item_id) {
    return ((ItemMetadata *) self->page_payload.bytes)[item_id.id];
}

// TODO: change public signature so user won't know anything about id and use just pointer
// maybe we don't even need this one
Result page_read_item(Page *self, item_index_t item_id, Item *item) {
    ASSERT_ARG_NOT_NULL(self);
    ASSERT_ARG_NOT_NULL(item);

    if (item_id.id >= self->page_header.next_item_id.id) {
        return ERROR("Item id is out of range");
    }

    // TODO: check this address magic in tests
    ItemMetadata metadata = get_metadata(self, item_id);
    *item = *(Item *) (self->page_payload.bytes + metadata.data_offset);

    return OK;
}

Result page_add_item(Page *self, Item *item, struct ItemResult *item_add_result) {
    ASSERT_ARG_NOT_NULL(self);
    ASSERT_ARG_NOT_NULL(item);

    // check that we have enough size to put item metadata and item data into free-range
    size_t free_space_size = self->page_header.free_space_end_offset - self->page_header.free_space_start_offset;
    if (item->size > free_space_size) {
        return ERROR("Not enough space to put item");
    }

    // we place data in the reverse order staring from page end
    int32_t data_offset = self->page_header.free_space_end_offset - item->size;
    item_index_t item_id = self->page_header.next_item_id;
    ItemMetadata metadata = {
            .id = item_id,
            .data_offset = (int32_t) data_offset,
            .size = item->size
    };

    *item_add_result = (struct ItemResult) {
            .metadata_offset_in_page = self->page_header.free_space_start_offset,
            .metadata = metadata,
            .item_id = item_id
    };
    self->page_header.free_space_start_offset =
            item_add_result->metadata_offset_in_page + (int32_t) sizeof(ItemMetadata);
    self->page_header.free_space_end_offset = data_offset;
    self->page_header.next_item_id.id++;

    return OK;
}

Result page_delete_item(Page *self, Item *item) {
    ASSERT_ARG_NOT_NULL(self);
    ASSERT_ARG_NOT_NULL(item);

    if (item->is_deleted) {
        return ERROR("Item is already deleted");
    }

    if (item->index_in_page.id >= self->page_header.next_item_id.id) {
        return ERROR("Item id is out of range");
    }

    ItemMetadata metadata = get_metadata(self, item->index_in_page);

    free(item->data);
    item->is_deleted = true;
    // if this was the last item in page then we can just move free space start offset
    //TODO: test this and check data consistency. Theoretically, we don't care about payload, but headers.
    // also, we should do defragmentation once in a time
    if (item->index_in_page.id == self->page_header.next_item_id.id - 1) {
        self->page_header.free_space_start_offset -= sizeof(ItemMetadata);
        self->page_header.free_space_end_offset += metadata.size;
        self->page_header.next_item_id.id--;
        *item = NULL_ITEM;
    }

    return OK;
}

size_t page_size(Page *self) {
    return self->page_header.page_size;
}