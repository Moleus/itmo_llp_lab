#include "private/storage/page_manager.h"

//TODO: think if it's correct to include private header
#include "private/storage/page.h"

// Page Iterator
PageIterator * page_iterator_new(PageManager *page_manager) {
    ASSERT_ARG_NOT_NULL(page_manager);

    PageIterator *result = malloc(sizeof(PageIterator));
    ASSERT_NOT_NULL(result, FAILED_TO_ALLOCATE_MEMORY);
    *result = (PageIterator) {.page_manager = page_manager, .next_page_id = 0,
                               //TODO: check
                               .current_page = page_manager->pages};
    return result;
}

void page_iterator_destroy(PageIterator *self) {
    ASSERT_ARG_NOT_NULL(self);
    free(self);
}

bool page_iterator_has_next(PageIterator *self) {
    ASSERT_ARG_NOT_NULL(self);

    int32_t pages_count = page_manager_get_pages_count(self->page_manager);
    return self->next_page_id.id < pages_count;
}

Result page_iterator_next(PageIterator *self, Page **result) {
    ASSERT_ARG_NOT_NULL(self);
    ASSERT_ARG_IS_NULL(*result);
    if (!page_iterator_has_next(self)) {
        ABORT_EXIT(INTERNAL_LIB_ERROR, "No more pages in iterator");
    }

    // TODO: check that it works
    Result get_page_res = page_manager_read_page(self->page_manager, self->next_page_id, result);
    RETURN_IF_FAIL(get_page_res, "Failed to get page by id");
    self->next_page_id.id++;
    self->current_page = *result;
    return OK;
}

// Item Iterator
ItemIterator * item_iterator_new(PageManager *page_manager) {
    ASSERT_ARG_NOT_NULL(page_manager);

    ItemIterator *item_it = malloc(sizeof(ItemIterator));
    ASSERT_NOT_NULL(item_it, FAILED_TO_ALLOCATE_MEMORY);

    PageIterator *page_iterator = page_manager_get_pages(page_manager);
    *item_it = (ItemIterator) {.page_iterator = page_iterator, .current_item = NULL, .current_item_index = -1};
    return item_it;
}

void item_iterator_destroy(ItemIterator *self) {
    ASSERT_ARG_NOT_NULL(self);
    page_iterator_destroy(self->page_iterator);
    free(self);
}

bool item_iterator_has_next(ItemIterator *self) {
    ASSERT_ARG_NOT_NULL(self);
    if (is_null_page(self->page_iterator->current_page->page_payload)) {
        // If next page exists then there must be at least one item on the next page
        return page_iterator_has_next(self->page_iterator);
    }

    if (next_item(self->current_item_index).id < self->page_iterator->current_page->page_header.next_item_id.id) {
        return true;
    }
    return page_iterator_has_next(self->page_iterator);
}

Result item_iterator_next(ItemIterator *self, Item **result) {
    ASSERT_ARG_NOT_NULL(self);
    ASSERT_ARG_IS_NULL(result);

    if (!item_iterator_has_next(self)) {
        ABORT_EXIT(INTERNAL_LIB_ERROR, "No more items in iterator");
    }

    if (self->current_item_index.id <= self->page_iterator->current_page->page_header.next_item_id.id) {
        // item is on next page
        page_index_t next_page_index = next_page(self->page_iterator->current_page->page_header.page_id);
        PageManager *pm = self->page_iterator->page_manager;
        // Здесь мы обращаемся к page_manager и просим у него следующую страницу.
        // Он уже должен определить - загружена ли она в память, или ее нужно достать с диска
        Page *page = NULL;
        Result res = page_manager_read_page(pm, next_page_index, &page);
        RETURN_IF_FAIL(res, "Failed to read item");
        //TODO: check increment
        self->current_item_index.id++;
        self->current_item = *result;
        return OK;
    }
    Page *page = NULL;
    // page is auto-incremented
    Result res = page_iterator_next(self->page_iterator, &page);
    RETURN_IF_FAIL(res, "Failed to get next_page page");
    // TODO: think about recursion. Theoretically it should be executed only once, so maybe no recursion needed
    return item_iterator_next(self, result);
}

// To top-level function which should be used to get items and pages
// TODO: do we need to get pages? If not - remove this method
PageIterator * page_manager_get_pages(PageManager *self) {
    ASSERT_ARG_NOT_NULL(self);

    return page_iterator_new(self);
}

ItemIterator * page_manager_get_items(PageManager *self) {
    ASSERT_ARG_NOT_NULL(self);

    return item_iterator_new(self);
}
