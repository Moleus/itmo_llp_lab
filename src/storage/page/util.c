#include "private/storage/page.h"

size_t page_size(Page *self) {
    return self->page_header.page_size;
}

uint32_t page_get_free_space_left(Page *self) {
    return self->page_header.free_space_end_offset - self->page_header.free_space_start_offset;
}

uint32_t page_get_payload_available_space(Page *self) {
    return page_get_free_space_left(self) - (uint32_t) sizeof(ItemMetadata);
}

uint32_t page_get_payload_size(uint32_t page_size) {
    return page_size - (uint32_t) HEADER_SIZE;
}

bool page_can_fit_payload(Page *self, uint32_t payload_size) {
    return page_get_payload_available_space(self) >= payload_size;
}

page_index_t page_get_item_continuation(Page *self, Item *item) {
    ASSERT_ARG_NOT_NULL(self)
    ASSERT_ARG_NOT_NULL(item)

    ItemMetadata *metadata = get_metadata(self, item->index_in_page);
    return metadata->continues_on_page;
}

ItemMetadata *get_metadata(const Page *self, item_index_t item_id) {
    return (ItemMetadata *) (self->page_payload + sizeof(ItemMetadata) * item_id.id);
}

uint8_t *get_item_data_addr(const Page *self, uint32_t data_offset) {
    return ((uint8_t *) self) + data_offset;
}

Item create_item(ItemPayload payload, item_index_t item_id) {
    return (Item) {.is_deleted = false, .index_in_page = item_id, .payload = payload};
}

page_index_t page_get_id(const Page *self) {
    return self->page_header.page_id;
}