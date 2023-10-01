#include "private/storage/page.h"
#include "private/storage/page_manager.h"

#define HEADER_SIZE sizeof(PageHeader)

Result page_item_new(Item *self, size_t id, int32_t size, void *data) {
    ASSERT_ARG_NOT_NULL(self);
    ASSERT_ARG_NOT_NULL(data);

    self->id = id;
    self->data = (Item) {.data = data, .size= size, .pos = 0}; // TODO: calc pos
    return OK;
}

Result page_item_destroy(Item *self) {
    ASSERT_ARG_NOT_NULL(self);
    ASSERT_NOT_NULL(self->data.data, PAGE_ITEM_DATA_IS_NULL);

    free(self->data.data);
    free(self);
    return OK;
}

size_t page_get_end_position(Page *self) {
    ASSERT_ARG_NOT_NULL(self);

    return self->page_header.page_size;
}

int32_t page_get_payload_size(int32_t page_size) {
    return page_size - (int32_t) HEADER_SIZE;
}

/*
 * Allocates new page. Assigns id
 */
Result page_new(int32_t page_id, int32_t page_size, Page *self) {
    ASSERT_ARG_IS_NULL(self);

    // TODO: rethink payload allocation
    u_int8_t *payload = malloc(page_get_payload_size(page_size));
    RETURN_IF_NULL(payload, "Failed to allocate page payload");
    self = malloc(sizeof(Page));
    RETURN_IF_NULL(self, "Failed to allocate page")

    PageHeader header = {
            .page_id = page_id,
            .file_offset = page_id * page_size,
            .free_space_start_offset = HEADER_SIZE,
            .free_space_end_offset = page_size
    };
    self->page_header = header;
    self->page_payload.bytes = payload;
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

Result page_read(Page *self, size_t item_id, Item *item) {
    ASSERT_ARG_NOT_NULL(self);
    ASSERT_ARG_NOT_NULL(item);

    if (item_id >= self->page_header.items_count) {
        return ERROR("Item id is out of range");
    }
    // TODO: implement
    return OK;
}

size_t page_size(Page* self) {
    return self->page_header.page_size;
}