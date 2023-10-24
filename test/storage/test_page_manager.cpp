#include "gtest/gtest.h"
#include "common.h"
#ifdef _WIN32
#include <cstdint>
#endif

extern "C" {
#include "private/storage/page_manager.h"
#include "public/util/common.h"
}

#define PAGE_SIZE 128
#define SIGNATURE 0x12345678

ItemPayload get_payload() {
    const char *data = "test data";
    ItemPayload payload = {
            .size = static_cast<uint32_t>(strlen(data)),
            .data = (void *) data
    };
    return payload;
}

// test page_manager. It should create page of size 128 bytes and write to it 8 bytes of data
TEST(test_page_manager, test_page_manager) {
    PageManager *pm = page_manager_new();
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
    ASSERT_EQ(result.metadata.item_id.item_id, 0);
    ASSERT_EQ(result.metadata.size, payload.size);
    ASSERT_EQ(result.metadata.data_offset, PAGE_SIZE - payload.size);
    ASSERT_EQ(result.metadata_offset_in_page, sizeof(PageHeader));

    page_manager_destroy(pm);
    pm = page_manager_new();
    res = page_manager_init(pm, FILE_PATH, PAGE_SIZE, SIGNATURE);
    page = page_manager_get_current_free_page(pm);
    ASSERT_EQ(res.status, RES_OK);
    page_manager_put_item(pm, page, payload, &result);
    ASSERT_EQ(result.metadata.item_id.item_id, 1);
    ASSERT_EQ(result.metadata.size, payload.size);
    ASSERT_EQ(result.metadata.data_offset, PAGE_SIZE - payload.size * 2);
    ASSERT_EQ(result.metadata_offset_in_page, sizeof(PageHeader) + sizeof(ItemMetadata));
    page_manager_destroy(pm);
    remove_file();
}

// add 2 items. Delete 1 item
TEST(test_page_manager, test_add_after_delete) {
    PageManager *pm = page_manager_new();
    page_manager_init(pm, FILE_PATH, PAGE_SIZE, SIGNATURE);

    ItemPayload payload = get_payload();
    ItemAddResult add_result1;
    ItemAddResult add_result2;
    page_manager_put_item(pm, page_manager_get_current_free_page(pm), payload, &add_result1);
    page_manager_put_item(pm, page_manager_get_current_free_page(pm), payload, &add_result2);
    Item item;
    uint8_t payload_buffer[payload.size];
    Result res = page_manager_get_item(pm, pm->current_free_page, add_result1.metadata.item_id, payload_buffer, &item);
    ASSERT_EQ(res.status, RES_OK);
    res = page_manager_delete_item(pm, pm->current_free_page, &item);
    ASSERT_EQ(res.status, RES_OK);
    ASSERT_EQ(pm->current_free_page->page_header.items_count, 1);
    ASSERT_EQ(pm->current_free_page->page_header.free_space_start_offset,
              sizeof(PageHeader) + sizeof(ItemMetadata) * 2);
    ASSERT_EQ(pm->current_free_page->page_header.free_space_end_offset, PAGE_SIZE - payload.size * 2);
    ASSERT_EQ(pm->current_free_page->page_header.next_item_id.item_id, 2);
    ASSERT_EQ(item.is_deleted, true);
    page_manager_destroy(pm);
    remove_file();
}

// add large item which is larger than page size. Check that it is split into 2 pages
TEST(test_page_manager, test_add_large_item) {
    PageManager *pm = page_manager_new();
    page_manager_init(pm, FILE_PATH, PAGE_SIZE, SIGNATURE);

    // create continues 0xFF data with size 128
    const uint32_t payload_size = PAGE_SIZE;
    uint8_t data[payload_size] = {0};
    memset(data, 0xFF, payload_size);

    ItemPayload payload = {
            .size = payload_size,
            .data = data
    };
    ItemAddResult add_result;
    uint32_t expected_bytes_written = PAGE_SIZE - sizeof(PageHeader) - sizeof(ItemMetadata);
    uint32_t expected_bytes_on_second_page = payload_size - expected_bytes_written;
    Page *first_page = page_manager_get_current_free_page(pm);
    Result res = page_manager_put_item(pm, first_page, payload, &add_result);
    ASSERT_EQ(res.status, RES_OK);
    ASSERT_EQ(add_result.write_status.complete, true);
    ASSERT_EQ(add_result.write_status.bytes_written, payload_size);
    ASSERT_EQ(add_result.metadata.continues_on_page.id, 1);
    ASSERT_EQ(add_result.metadata.size, expected_bytes_written);
    ASSERT_EQ(add_result.metadata.item_id.item_id, 0);
    ASSERT_EQ(first_page->page_header.items_count, 1);
    ASSERT_EQ(first_page->page_header.next_item_id.item_id, 1);

    Page *second_page = page_manager_get_current_free_page(pm);
    ASSERT_EQ(second_page->page_header.page_id.id, 1);
    ASSERT_EQ(second_page->page_header.items_count, 1);
    ASSERT_EQ(second_page->page_header.next_item_id.item_id, 1);
    ASSERT_EQ(second_page->page_header.free_space_start_offset, sizeof(PageHeader) + sizeof(ItemMetadata));
    ASSERT_EQ(second_page->page_header.free_space_end_offset, PAGE_SIZE - expected_bytes_on_second_page);
    page_manager_destroy(pm);
    remove_file();
}