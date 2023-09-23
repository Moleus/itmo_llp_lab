#include "private/storage/page.h"

#define PAGE_SIZE 4096

Result page_item_new(PageItem *self, size_t id, int32_t size, void *data) {
    ASSERT_ARG_NOT_NULL(self);
    ASSERT_ARG_NOT_NULL(data);

    self->id = id;
    self->data = (Value) {.data = data, .len = size, .pos = 0}; // TODO: calc pos
    return OK;
}

Result page_item_destroy(PageItem *self) {
    ASSERT_ARG_NOT_NULL(self);
    ASSERT_NOT_NULL(self->data.data, PAGE_ITEM_DATA_IS_NULL);

    free(self->data.data);
    free(self);
    return OK;
}

size_t page_get_end_position(Page *self) {
    ASSERT_ARG_NOT_NULL(self);

    return self->page_header.used_bytes;
}

/*
 * Allocates new page. Assigns id
 */
Result page_new(int32_t page_id, Page *self) {
    ASSERT_ARG_IS_NULL(self);

    self = malloc(sizeof(Page));
    RETURN_IF_NULL(self, "Failed to allocate page")

    self->page_id = page_id;
    self->offset = page_id * PAGE_SIZE;
    PageHeader header = {.used_bytes = 0, .status = PAGE_STATUS_UNUSED};
    self->page_header = header;
    return OK;
}

Result page_destroy(Page *self) {
    ASSERT_ARG_NOT_NULL(self);

    free(self);
    return OK;
}

int32_t page_get_data_offset() {
    return sizeof(PageHeader);
}

size_t page_size() {
    return PAGE_SIZE;
}