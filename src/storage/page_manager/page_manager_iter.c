#include "private/storage/page_manager.h"

//TODO: think if it's correct to include private header
#include "private/storage/page.h"

// Page Iterator
PageIterator *page_iterator_new(PageManager *self) {
    ASSERT_ARG_NOT_NULL(self)

    PageIterator *result = malloc(sizeof(PageIterator));
    ASSERT_NOT_NULL(result, FAILED_TO_ALLOCATE_MEMORY)
    // начинаем с первой страницы
    Page *page = NULL;
    if (page_manager_get_pages_count(self) > 0) {
        page_manager_read_page(self, page_id(0), &page);
    }
    *result = (PageIterator) {.page_manager = self, .next_page_id = 0,
            // TODO: check page for null while iterating
            .current_page = page};
    return result;
}

void page_iterator_destroy(PageIterator *self) {
    ASSERT_ARG_NOT_NULL(self)
    free(self);
}

bool page_iterator_has_next(PageIterator *self) {
    ASSERT_ARG_NOT_NULL(self)

    uint32_t pages_count = page_manager_get_pages_count(self->page_manager);
    return self->next_page_id.id < pages_count;
}

Result page_iterator_next(PageIterator *self, Page **result) {
    ASSERT_ARG_NOT_NULL(self)
    ASSERT_ARG_IS_NULL(*result)
    if (!page_iterator_has_next(self)) {
        ABORT_EXIT(INTERNAL_LIB_ERROR, "No more pages in iterator")
    }

    // TODO: check that it works
    Result get_page_res = page_manager_read_page(self->page_manager, self->next_page_id, result);
    RETURN_IF_FAIL(get_page_res, "Failed to get page by id")
    self->next_page_id.id++;
    self->current_page = *result;
    return OK;
}

// Item Iterator
ItemIterator *item_iterator_new(PageManager *page_manager) {
    ASSERT_ARG_NOT_NULL(page_manager)

    ItemIterator *item_it = malloc(sizeof(ItemIterator));
    ASSERT_NOT_NULL(item_it, FAILED_TO_ALLOCATE_MEMORY)

    PageIterator *page_iterator = page_manager_get_pages(page_manager);
    *item_it = (ItemIterator) {.page_iterator = page_iterator, .current_item = NULL, .current_item_index = -1};
    return item_it;
}

void item_iterator_destroy(ItemIterator *self) {
    ASSERT_ARG_NOT_NULL(self)
    page_iterator_destroy(self->page_iterator);
    free(self);
}

bool item_iterator_has_next(ItemIterator *self) {
    ASSERT_ARG_NOT_NULL(self)
    Page *cur_page = self->page_iterator->current_page;

    // Проверка когда нет первой страницы
    if (cur_page == NULL) {
        // there are no pages in file
        LOG_DEBUG("Current page is null. Total pages: %d", page_manager_get_pages_count(self->page_iterator->page_manager));
        return false;
    }
    // Если текущая страница не пустая
    if (next_item(self->current_item_index).id < cur_page->page_header.next_item_id.id) {
        // TODO: работает ли это, когда мы удаляем айтемы со страницы?
        // Скорее всего да, т.к за next_item_id не должно быть удаленных элементов
        return true;
    }
    // Если на этой странице больше нет элементов - надо проверить на следующей
    // нужно загрузить следующую страницу и проверить, что кол-во элементов в ней больше 0
    // надо проверять следующие страницы, пока не найдем страницу с элементами или кол-во страниц не закончится
    // Также, нужно проверить, что элемент - это НЕ продолжение предыдущего элемента
    LOG_DEBUG("ItemIterator - checking next page %d", self->page_iterator->next_page_id.id);
    while (page_iterator_has_next(self->page_iterator)) {
        cur_page = NULL;
        Result res = page_iterator_next(self->page_iterator, &cur_page);
        ABORT_IF_FAIL(res, "Failed to get next page")
        ItemMetadata *cur_item_metadata = get_metadata(cur_page, self->current_item_index);
        if (cur_page->page_header.items_count > 0) {
            if (cur_page->page_header.items_count == 1 && cur_item_metadata->continues_on_page.id == page_get_id(cur_page).id) {
                // Если на странице только 1 элемент и он продолжается на следующей странице - то это не отдельный элемент
                LOG_DEBUG("ItemIterator - Page %d has only one element and it's continuation of %d", page_get_id(cur_page).id,
                          self->current_item_index.id);
                continue;
            }
            LOG_DEBUG("ItemIterator - found item on page %d", cur_page->page_header.page_id.id);
            return true;
        }
    }
    return false;
}

Result item_iterator_next(ItemIterator *self, Item **result) {
    ASSERT_ARG_NOT_NULL(self)
    ASSERT_ARG_IS_NULL(*result)

    if (!item_iterator_has_next(self)) {
        ABORT_EXIT(INTERNAL_LIB_ERROR, "No more items in iterator")
    }

    item_index_t old_item_index = self->current_item_index;
    Page *cur_page = self->page_iterator->current_page;
    int32_t new_item_index = ++self->current_item_index.id;

    if (new_item_index > cur_page->page_header.next_item_id.id) {
        LOG_ERR("Page: %d. Next item %d is on next page", cur_page, new_item_index);
        ABORT_EXIT(INTERNAL_LIB_ERROR, "It should not be possible because has_next sets current_page or returns false")
    }

    Result res = page_manager_get_item(self->page_iterator->page_manager, cur_page, next_item(old_item_index), *result);
    RETURN_IF_FAIL(res, "Failed to get item from page")
    self->current_item = *result;
    return OK;
}

// To top-level function which should be used to get items and pages
// TODO: do we need to get pages? If not - remove this method
PageIterator *page_manager_get_pages(PageManager *self) {
    ASSERT_ARG_NOT_NULL(self)

    return page_iterator_new(self);
}

ItemIterator *page_manager_get_items(PageManager *self) {
    ASSERT_ARG_NOT_NULL(self)

    return item_iterator_new(self);
}
