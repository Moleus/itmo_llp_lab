#include "private/storage/page_manager.h"

//TODO: think if it's correct to include private header
#include "private/storage/page.h"

// Page Iterator
Result page_iterator_new(PageManager *page_manager, PageIterator **result) {
    ASSERT_ARG_NOT_NULL(page_manager);
    ASSERT_ARG_IS_NULL(result);

    *result = malloc(sizeof(PageIterator));
    **result = (PageIterator) {.page_manager = page_manager, .next_page_id = 0,
                               //TODO: check
                               .current_page = *page_manager->pages};
    return OK;
}

void page_iterator_destroy(PageIterator *self) {
    ASSERT_ARG_NOT_NULL(self);
    free(self);
}

bool page_iterator_has_next(PageIterator *self) {
    ASSERT_ARG_NOT_NULL(self);

    return self->next_page_id.id < self->page_manager->pages_count;
}

Result page_iterator_next(PageIterator *self, Page *result) {
    ASSERT_ARG_NOT_NULL(self);
    ASSERT_ARG_NOT_NULL(result);
    if (!page_iterator_has_next(self)) {
        return ERROR("No more pages");
    }

    // TODO: check that it works
    Result get_page_res = page_manager_read_page(self->page_manager, self->next_page_id, result);
    RETURN_IF_FAIL(get_page_res, "Failed to get page by id");
    self->next_page_id.id++;
    self->current_page = *result;
    return OK;
}

// Item Iterator
Result item_iterator_new(PageManager *page_manager, ItemIterator **result) {
    ASSERT_ARG_NOT_NULL(page_manager);
    ASSERT_ARG_IS_NULL(result);

    *result = malloc(sizeof(ItemIterator));
    PageIterator *page_iterator;
    Result res = page_manager_get_pages(page_manager, &page_iterator);
    RETURN_IF_FAIL(res, "Failed to get pages iterator");
    **result = (ItemIterator) {.page_iterator = page_iterator, .current_item = NULL_ITEM, .current_item_index = 0};
    return OK;
}

void item_iterator_destroy(ItemIterator *self) {
    ASSERT_ARG_NOT_NULL(self);
    page_iterator_destroy(self->page_iterator);
    free(self);
}

bool item_iterator_has_next(ItemIterator *self) {
    ASSERT_ARG_NOT_NULL(self);
    if (is_null_page(self->page_iterator->current_page.page_payload)) {
        // If next page exists then there must be at least one item on the next page
        return page_iterator_has_next(self->page_iterator);
    }

    if (self->current_item_index <= self->page_iterator->current_page.page_header.next_item_id) {
        return true;
    }
    return page_iterator_has_next(self->page_iterator);
}

Result item_iterator_next(ItemIterator *self, Item *result) {
    ASSERT_ARG_NOT_NULL(self);
    ASSERT_ARG_IS_NULL(result);

    if (!item_iterator_has_next(self)) {
        return ERROR("No more items");
    }

    if (self->current_item_index.id <= self->page_iterator->current_page.page_header.next_item_id.id) {
        page_index_t next_page_index = next_page(self->page_iterator->current_page.page_header.page_id);
        PageManager *pm = self->page_iterator->page_manager;
        // Здесь мы обращаемся к page_manager и просим у него следующую страницу.
        // Он уже должен определить - загружена ли она в память, или ее нужно достать с диска
        Page *page;
        Result res = page_manager_read_page(pm, next_page_index, page);
        RETURN_IF_FAIL(res, "Failed to read item");
        //TODO: check increment
        self->current_item_index.id++;
        self->current_item = *result;
        return OK;
    }
    Page page;
    // page is auto-incremented
    Result res = page_iterator_next(self->page_iterator, &page);
    RETURN_IF_FAIL(res, "Failed to get next_page page");
    // TODO: think about recursion. Theoretically it should be executed only once, so maybe no recursion needed
    return item_iterator_next(self, result);
}

// To top-level function which should be used to get items and pages
// TODO: do we need to get pages? If not - remove this method
Result page_manager_get_pages(PageManager *self, PageIterator **result) {
    ASSERT_ARG_NOT_NULL(self);
    ASSERT_ARG_IS_NULL(result);

    return page_iterator_new(self, result);
}

Result page_manager_get_items(PageManager *self, ItemIterator **result) {
    ASSERT_ARG_NOT_NULL(self);
    ASSERT_ARG_IS_NULL(result);

    return item_iterator_new(self, result);
}
