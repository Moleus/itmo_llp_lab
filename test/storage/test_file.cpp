#include "gtest/gtest.h"
#include "gmock/gmock.h"

extern "C" {
#include "public/storage/file.h"
}

#define FILE_PATH "test.llp"

void assert_ok(Result res) {
    Result expected = (Result) {.status = RES_OK, .message = nullptr};
    ASSERT_STREQ(res.message, expected.message);
    ASSERT_EQ(res.status, expected.status);
}

TEST(test_file, test_file_new) {
    FileState *file = file_new();
    ASSERT_EQ(file_is_open(file), false);
    file_destroy(file);
}

TEST(test_file, test_file_open) {
    FileState *file = file_new();
    Result res = file_open(file, FILE_PATH);
    assert_ok(res);
    ASSERT_EQ(file_is_open(file), true);
    res = file_close(file);
    assert_ok(res);
    ASSERT_EQ(file_is_open(file), false);
    file_destroy(file);
    remove(FILE_PATH);
}

TEST(test_file, test_file_read) {
    FileState *file = file_new();
    // manually write to file
    FILE *fd = fopen(FILE_PATH, "w");
    fwrite("test", strlen("test"), 1, fd);
    fclose(fd);
    char buf[strlen("st")];
    file_open(file, FILE_PATH);
    Result res = file_read(file, buf, 2, strlen("st"));
    assert_ok(res);
    ASSERT_EQ(buf[0], 's');
    ASSERT_EQ(buf[1], 't');
    res = file_close(file);
    assert_ok(res);
    file_destroy(file);
    remove(FILE_PATH);
}

TEST(test_file, test_file_write) {
    FileState *file = file_new();
    file_open(file, FILE_PATH);
    uint8_t buf[] = {0xBE, 0xAF, 0xBA, 0xBE};
    Result res = file_write(file, buf, 0, sizeof(buf));
    assert_ok(res);
    uint8_t buf_new[] = {0xFF, 0xFE};
    res = file_write(file, buf_new, 2, 2);
    assert_ok(res);
    uint8_t buf_res[4];
    res = file_read(file, buf_res, 0, sizeof(buf));
    assert_ok(res);
    uint8_t buf_expected[4] = {0xBE, 0xAF, 0xFF, 0xFE};
    ASSERT_THAT(buf_res, ::testing::ElementsAreArray(buf_expected));
    res = file_close(file);
    assert_ok(res);
    file_destroy(file);
    remove(FILE_PATH);
}