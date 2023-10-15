#include "private/storage/page_manager.h"

//TODO: think if it's correct to include private header
#include "private/storage/page.h"
#include "private/storage/file_manager.h"

/*
 * Page should exist in memory or on disk
 * If page doesn't exist in memory then it will be loaded from disk
 */
Result page_manager_read_page(PageManager *self, page_index_t id, Page **result_page) {
    ASSERT_ARG_NOT_NULL(self)
    ASSERT_ARG_IS_NULL(*result_page)

    // NOTE: здесь мы выдаем ошибку, если страницы не существует.
    // Если мы хотим получить страницу и не знаем, есть она или нет -
    uint32_t pages_count = page_manager_get_pages_count(self);
    if (id.id >= pages_count) {
        ABORT_EXIT(INTERNAL_LIB_ERROR, "Page doesn't exist")
    }

    // check if page exists in ram
    Result res = page_manager_get_page_from_ram(self, id, result_page);
    if (res.status == RES_OK) {
        LOG_INFO("Page %d found in ram", id.id);
        // page found in ram
        return OK;
    }

    // TODO: can we use page_manager_page_new instead?
    // no. Because here we get page by id. And new only allocates new page

    // pass offset to file_manger and get page
    size_t page_offset_in_file = page_manager_get_page_offset(self, id);
    // load from disk
    // allocate page here
    uint32_t page_size = page_manager_get_page_size(self);
    Page *page = page_new(id, page_size);
    // read header
    // TODO: check page reference pointer
    res = file_manager_read(self->file_manager, page_offset_in_file, page_size, page);
    RETURN_IF_FAIL(res, "Failed to read page header from file")
    // read payload
    LOG_DEBUG("Read page %d. Offset: %08X, items count: %d, free space: ", id.id,
              page_offset_in_file, page->page_header.items_count,
              page->page_header.free_space_end_offset - page->page_header.free_space_start_offset);

    // TODO: don't forget about this page in memory
    // TODO: возможная проблема. Каким-то образом, в связном списке одна страница может оказаться дважды.
    // Если мы будем вычитывать страницу с диска несколько раз.
    page_manager_add_page_to_cache(self, page);
    *result_page = page;
    // TODO: удалять страницу из памяти, после использования.

    return OK;
}
