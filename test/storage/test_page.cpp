#include <gmock/gmock-matchers.h>
#include "gtest/gtest.h"

extern "C" {
    #include "private//storage/page.h"
}

TEST(test_page, page_add_item) {
    Page *page = NULL;
    page_index_t id = page_id(0);
    int32_t size = 1024;
    Result res = page_new(id, size);
    EXPECT_STREQ(res.message, nullptr);
    ASSERT_EQ(res.status, RES_OK);
    EXPECT_EQ(page->page_header.page_id.id, id.id);
    EXPECT_EQ(page->page_header.file_offset, 0);

    ItemPayload payload;
    const char *data = "test data";
    int32_t payload_size = (int32_t) strlen(data);
    payload.size = payload_size;
    payload.data = (void*) data;
    ItemAddResult result;
    res = page_add_item(page, payload, &result);
    EXPECT_STREQ(res.message, nullptr);
    ASSERT_EQ(res.status, RES_OK);

    const ItemAddResult expected = {
        .metadata_offset_in_page = sizeof(PageHeader),
        .metadata = {
            .item_id = item_id(0),
            .data_offset = size - payload_size,
            .size = payload_size
        },
        .item_id = item_id(0)
    };
    EXPECT_EQ(result.metadata_offset_in_page, expected.metadata_offset_in_page);
    EXPECT_EQ(result.metadata.item_id.id, expected.metadata.item_id.id);
    EXPECT_EQ(result.metadata.data_offset, expected.metadata.data_offset);
    EXPECT_EQ(result.metadata.size, expected.metadata.size);
    EXPECT_EQ(result.item_id.id, expected.item_id.id);

    res = page_destroy(page);
    EXPECT_STREQ(res.message, nullptr);
    ASSERT_EQ(res.status, RES_OK);
}