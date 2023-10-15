#include "private/storage/page_manager.h"


#define MINIMUM_DATA_PART_SIZE_BYTES 8

Result page_manager_get_page_for_data(PageManager *self, ItemPayload data, Page **result) {
    // get data size. Calculate size with metadata.
    // Bad: Iterate over all pages and find page which has enough space
    // Good: Take page for O(1)

    // page_manager should keep track of current free page.
    // If payload doesn't fit - it should return new allocated page.

    Page *current_free_page = page_manager_get_current_free_page(self);
    uint32_t available_space = page_get_payload_available_space(current_free_page);
    if (available_space < MINIMUM_DATA_PART_SIZE_BYTES) {
        // can't place item on this page allocate new page
        Page *new_page = NULL;
        page_manager_page_new(self, &new_page);
        page_manager_set_current_free_page(self, new_page);
        *result = new_page;
        return OK;
    }
    *result = current_free_page;
    return OK;
}
