#include <assert.h>
#include "private/storage/page_manager.h"

//TODO: think if it's correct to include private header
#include "private/storage/page.h"

PageIterator *page_iterator_new(PageManager *self) {
    ASSERT_ARG_NOT_NULL(self)

    PageIterator *result = malloc(sizeof(PageIterator));
    ASSERT_NOT_NULL(result, FAILED_TO_ALLOCATE_MEMORY)
    // начинаем с первой страницы
    Page *page = NULL;
    assert(page_manager_get_pages_count(self) > 0);
    page_manager_read_page(self, page_id(0), &page);
    *result = (PageIterator) {.page_manager = self, .next_page_id = {1},
            .current_page = page};
    return result;
}

void page_iterator_destroy(PageIterator *self) {
    ASSERT_ARG_NOT_NULL(self)
    page_manager_free_pages(self->page_manager);
    free(self);
}

bool page_iterator_has_next(PageIterator *self) {
    ASSERT_ARG_NOT_NULL(self)

    uint32_t pages_count = page_manager_get_pages_count(self->page_manager);
    return self->next_page_id.id < (int32_t) pages_count;
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
ItemIterator *item_iterator_new(PageManager *page_manager, Item *reusable_memory) {
    ASSERT_ARG_NOT_NULL(page_manager)

    ItemIterator *item_it = malloc(sizeof(ItemIterator));
    ASSERT_NOT_NULL(item_it, FAILED_TO_ALLOCATE_MEMORY)

    struct AllocatedPayload *allocated_payloads = calloc(1, sizeof(struct AllocatedPayload));

    PageIterator *page_iterator = page_manager_get_pages(page_manager);
    *reusable_memory = (Item) {.is_deleted = true, .index_in_page.id = -1, .payload = {.data = NULL, .size = 0}};
    *item_it = (ItemIterator) {.page_iterator = page_iterator, .current_item = reusable_memory, .current_item_index = {-1}, .allocated_payloads = allocated_payloads, .allocated_payloads_count = 0};
    return item_it;
}

void item_iterator_destroy(ItemIterator *self) {
    ASSERT_ARG_NOT_NULL(self)
    item_iterator_free_payloads(self);
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
    // проблема, когда мы нашли следующую страницу и перешли на нее. Тогда наш индекс больше чем следующий айтем
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
            // если мы нашли следующую страницу, то обнуляем индес
            self->current_item_index.id = -1;
            return true;
        }
    }
    return false;
}

uint8_t *item_iterator_allocate_payload(ItemIterator *self, size_t size) {
    ASSERT_ARG_NOT_NULL(self)

    uint8_t *payload = calloc(1, size);
    ASSERT_NOT_NULL(payload, FAILED_TO_ALLOCATE_MEMORY)

    struct AllocatedPayload *allocated_payload = calloc(1, sizeof(struct AllocatedPayload));
    ASSERT_NOT_NULL(allocated_payload, FAILED_TO_ALLOCATE_MEMORY)

    allocated_payload->payload = payload;
    allocated_payload->next = self->allocated_payloads;
    self->allocated_payloads = allocated_payload;
    self->allocated_payloads_count++;

    return payload;
}

void item_iterator_free_payloads(ItemIterator *self) {
    ASSERT_ARG_NOT_NULL(self)

    uint32_t i = 0;
    struct AllocatedPayload *allocated_payload = self->allocated_payloads;
    while (allocated_payload != NULL) {
        i++;
        struct AllocatedPayload *next = allocated_payload->next;
        free(allocated_payload->payload);
        free(allocated_payload);
        allocated_payload = next;
    }
    LOG_DEBUG("ItemIterator - freed %d payloads. Count was %d", i, self->allocated_payloads_count);
    self->allocated_payloads_count = 0;
    self->allocated_payloads = NULL;
}

Result item_iterator_next(ItemIterator *self, Item *result) {
    ASSERT_ARG_NOT_NULL(self)
    ASSERT_ARG_NOT_NULL(result)

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

    uint32_t payload_size;
    Result res = page_manager_calculate_large_item_size(self->page_iterator->page_manager, cur_page,
                                                                   next_item(old_item_index), &payload_size);
    RETURN_IF_FAIL(res, "Failed to calculate large item size");
    uint8_t *buffer = item_iterator_allocate_payload(self, payload_size);

    res = page_manager_get_item(self->page_iterator->page_manager, cur_page, next_item(old_item_index), buffer,
                                       result);
    assert(result->payload.size == payload_size);
    RETURN_IF_FAIL(res, "Failed to get item from page")
    return OK;
}

// To top-level function which should be used to get items and pages
// TODO: do we need to get pages? If not - remove this method
PageIterator *page_manager_get_pages(PageManager *self) {
    ASSERT_ARG_NOT_NULL(self)

    return page_iterator_new(self);
}

ItemIterator *page_manager_get_items(PageManager *self, Item *reusable_mem) {
    ASSERT_ARG_NOT_NULL(self)
    ASSERT_ARG_NOT_NULL(reusable_mem)

    return item_iterator_new(self, reusable_mem);
}
