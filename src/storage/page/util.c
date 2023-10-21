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