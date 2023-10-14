#include "private/storage/page_manager.h"


Result page_manager_get_page_for_data(PageManager *self, ItemPayload data, Page **result) {
    // get data size. Calculate size with metadata.
    // Bad: Iterate over all pages and find page which has enough space
    // Good: Take page for O(1)

    // page_manager should keep track of current free page.
    // If payload doesn't fit - it should return new allocated page.
    *result = self->current_free_page;
    return OK;
}
