#include "gtest/gtest.h"

extern "C" {
    #include "private/storage/page_manager.h"
    #include "public/util/common.h"
}

#define PAGE_SIZE 128
#define FILE_PATH "/tmp/test_page_manager.txt"
#define SIGNATURE 0x12345678

ItemPayload get_payload() {
    ItemPayload payload = {
            .data = (uint8_t *) "\x00\x01\x02\x03\x04\x05\x06\x07",
            .size = 8
    };
    return payload;
}

// test page_manager. It should create page of size 128 bytes and write to it 8 bytes of data
TEST(test_page_manager, test_page_manager) {
    PageManager *pm = page_manager_new();
    remove(FILE_PATH);
    Result res = page_manager_init(pm, FILE_PATH, PAGE_SIZE, SIGNATURE);
    ASSERT_EQ(res.status, RES_OK);

    ASSERT_EQ(page_manager_get_pages_count(pm), 1);
    ASSERT_EQ(page_manager_get_page_size(pm), PAGE_SIZE);
    ASSERT_EQ(page_manager_get_current_free_page(pm)->page_header.page_id.id, 0);
    ASSERT_EQ(page_manager_get_last_page_id(pm).id, 0);
    Page *page = page_manager_get_current_free_page(pm);
    ItemPayload payload = get_payload();
    ItemAddResult result;
    page_manager_put_item(pm, page, payload, &result);
    ASSERT_EQ(result.metadata.item_id.id, 0);
    ASSERT_EQ(result.metadata.size, 8);
    ASSERT_EQ(result.metadata.data_offset, PAGE_SIZE - 8);
    ASSERT_EQ(result.metadata_offset_in_page, sizeof(PageHeader));

    page_manager_destroy(pm);
    pm = page_manager_new();
    res = page_manager_init(pm, FILE_PATH, PAGE_SIZE, SIGNATURE);
    page = page_manager_get_current_free_page(pm);
    ASSERT_EQ(res.status, RES_OK);
    page_manager_put_item(pm, page, payload, &result);
    ASSERT_EQ(result.metadata.item_id.id, 1);
    ASSERT_EQ(result.metadata.size, 8);
    ASSERT_EQ(result.metadata.data_offset, PAGE_SIZE - 8 * 2);
    ASSERT_EQ(result.metadata_offset_in_page, sizeof(PageHeader) + sizeof(ItemMetadata));
}

// add 2 items. Delete 1 item
TEST(test_page_manager, test_add_after_delete) {
    PageManager *pm = page_manager_new();
    remove(FILE_PATH);
    page_manager_init(pm, FILE_PATH, PAGE_SIZE, SIGNATURE);

    ItemPayload payload = get_payload();
    ItemAddResult add_result1;
    ItemAddResult add_result2;
    page_manager_put_item(pm, page_manager_get_current_free_page(pm), payload, &add_result1);
    page_manager_put_item(pm, page_manager_get_current_free_page(pm), payload, &add_result2);
    Item *item = nullptr;
    Result res = page_get_item(pm->current_free_page, add_result1.metadata.item_id, &item);
    ASSERT_EQ(res.status, RES_OK);
    res = page_manager_delete_item(pm, pm->current_free_page, item);
    ASSERT_EQ(res.status, RES_OK);
    ASSERT_EQ(pm->current_free_page->page_header.items_count, 1);
    ASSERT_EQ(pm->current_free_page->page_header.free_space_start_offset, sizeof(PageHeader) + sizeof(ItemMetadata) * 2);
    ASSERT_EQ(pm->current_free_page->page_header.free_space_end_offset, PAGE_SIZE - 8 * 2);
    ASSERT_EQ(pm->current_free_page->page_header.next_item_id.id, 2);
    page_get_item(pm->current_free_page, add_result1.metadata.item_id, &item);
    ASSERT_EQ(item->is_deleted, true);
}