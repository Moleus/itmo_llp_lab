#include "private/storage/storage.h"


Storage* storage_new(PageManager *page_manager) {
    ASSERT_ARG_NOT_NULL(page_manager);

    Storage *storage = malloc(sizeof(Storage));
    ASSERT_NOT_NULL(storage, FAILED_TO_ALLOCATE_MEMORY);
    storage->page_manager = page_manager;
    return storage;
}