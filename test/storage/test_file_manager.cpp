#include "gtest/gtest.h"

extern "C" {
    #include "private/storage/file_manager.h"
}

TEST(test_file_manager, test_file_manager_new) {
    FileManager *fm = file_manager_new();
    ASSERT_NE(fm->file, nullptr);
    file_manager_destroy(fm);
}