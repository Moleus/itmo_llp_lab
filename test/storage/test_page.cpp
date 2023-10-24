#include <gmock/gmock-matchers.h>
#include "gtest/gtest.h"

extern "C" {
    #include "private//storage/page.h"
}

TEST(test_page, page_add_item) {
    page_index_t id = page_id(0);
    uint32_t size = 1024;
    Page *page = page_new(id, size);
    EXPECT_EQ(page->page_header.page_id.id, id.id);

    ItemPayload payload;
    const char *data = "test data";
    auto payload_size = (uint32_t) strlen(data);
    payload.size = payload_size;
    payload.data = (void*) data;
    ItemAddResult result;
    Result res = page_add_item(page, payload, NULL_PAGE_INDEX, &result);
    EXPECT_STREQ(res.message, nullptr);
    ASSERT_EQ(res.status, RES_OK);

    const ItemAddResult expected = {
        .metadata_offset_in_page = sizeof(PageHeader),
        .metadata = {
            .item_id = item_id(page->page_header.page_id, 0),
            .data_offset = size - payload_size,
            .size = payload_size
        },
    };
    EXPECT_EQ(result.metadata_offset_in_page, expected.metadata_offset_in_page);
    EXPECT_EQ(result.metadata.item_id.item_id, expected.metadata.item_id.item_id);
    EXPECT_EQ(result.metadata.data_offset, expected.metadata.data_offset);
    EXPECT_EQ(result.metadata.size, expected.metadata.size);

    page_destroy(page);
    EXPECT_STREQ(res.message, nullptr);
    ASSERT_EQ(res.status, RES_OK);
}