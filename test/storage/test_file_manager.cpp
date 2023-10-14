#include "gtest/gtest.h"

extern "C" {
    #include "private/storage/file_manager.h"
}

TEST(test_file_manager, test_file_manager_new) {
    FileManager *fm = nullptr;
    Result res = file_manager_new();
    Result expected = (Result){.status = RES_OK, .message = nullptr};
    ASSERT_STREQ(res.message, expected.message);
    ASSERT_EQ(res.status, expected.status);
    ASSERT_NE(fm->file, nullptr);
    res = file_manager_destroy(fm);
    ASSERT_STREQ(res.message, expected.message);
    ASSERT_EQ(res.status, expected.status);
}