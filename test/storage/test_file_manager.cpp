#include "gtest/gtest.h"

extern "C" {
    #include "private/storage/file_manager.h"
}

TEST(test_file_manager, test_file_manager_new) {
    FileManager *fm = file_manager_new();
    ASSERT_NE(fm->file, nullptr);
    file_manager_destroy(fm);
}

TEST(test_file_manager, test_file_manager_init) {
    FileManager *fm = file_manager_new();
    const char *filename = "/tmp/test_file_manager.txt";
    remove(filename);
    FileHeaderConstants header_for_new_file = {
            .signature = 0x12345678,
            .page_size = 512
    };
    Result res = file_manager_init(fm, filename, header_for_new_file);
    ASSERT_EQ(res.status, RES_OK);
    ASSERT_EQ(fm->file->is_new, true);
    ASSERT_EQ(fm->file->size, 0);
    ASSERT_EQ(fm->header.constants.signature, header_for_new_file.signature);
    ASSERT_EQ(fm->header.constants.page_size, header_for_new_file.page_size);
    ASSERT_EQ(fm->header.dynamic.file_size, 0);
    ASSERT_EQ(fm->header.dynamic.current_free_page, 0);
    ASSERT_EQ(fm->header.dynamic.page_count, 0);
    file_manager_destroy(fm);
}

TEST(test_file_manager, test_file_write_header) {
    FileManager *fm = file_manager_new();
    const char *filename = "/tmp/test_file_manager.txt";
    remove(filename);
    FileHeaderConstants header_for_new_file = {
            .signature = 0x12345678,
            .page_size = 512
    };
    Result res = file_manager_init(fm, filename, header_for_new_file);
    ASSERT_EQ(res.status, RES_OK);
    fm->header.dynamic.current_free_page = 1;
    res = file_manager_write_header(fm);
    ASSERT_EQ(res.status, RES_OK);
    fm->header.dynamic.current_free_page = 12312;
    res = file_manager_read_header(fm);
    ASSERT_EQ(res.status, RES_OK);
    ASSERT_EQ(fm->header.dynamic.current_free_page, 1);
    file_manager_destroy(fm);
}

// test reopen file and read header
TEST(test_file_manager, test_file_manager_reopen) {
    FileManager *fm = file_manager_new();
    const char *filename = "/tmp/test_file_manager.txt";
    remove(filename);
    FileHeaderConstants header_for_new_file = {
            .signature = 0x12345678,
            .page_size = 512
    };
    Result res = file_manager_init(fm, filename, header_for_new_file);
    ASSERT_EQ(res.status, RES_OK);
    fm->header.dynamic.current_free_page = 1;
    res = file_manager_write_header(fm);
    ASSERT_EQ(res.status, RES_OK);
    file_manager_destroy(fm);

    fm = file_manager_new();
    res = file_manager_init(fm, filename, header_for_new_file);
    ASSERT_EQ(res.status, RES_OK);
    ASSERT_EQ(fm->header.dynamic.current_free_page, 1);
    file_manager_destroy(fm);
}