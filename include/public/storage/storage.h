// storage - interface to add/update/delete items in memory. It accepts *void raw_data and
// calls methods from page_manager/file_manager to persist item in db.

#include "private/storage/page_manager.h"

typedef struct {
    PageManager *page_manager;
} Storage;


Storage* storage_new(PageManager *page_manager);
