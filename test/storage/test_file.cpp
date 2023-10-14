#include "gtest/gtest.h"
#include "gmock/gmock.h"

extern "C" {
    #include "public/storage/file.h"
}

void assert_ok(Result res) {
    Result expected = (Result){.status = RES_OK, .message = nullptr};
    ASSERT_STREQ(res.message, expected.message);
    ASSERT_EQ(res.status, expected.status);
}

TEST(test_file, test_file_new) {
    FileState *file = nullptr;
    Result res = file_new();
    Result expected = (Result){.status = RES_OK, .message = nullptr};
    assert_ok(res);
    ASSERT_EQ(file_is_open(file), false);
    file_destroy(file);
};

TEST(test_file, test_file_open) {
    FileState *file = nullptr;
    file_new();
    Result res = file_open(file, "/tmp/test_file.txt");
    assert_ok(res);
    ASSERT_EQ(file_is_open(file), true);
    res = file_close(file);
    assert_ok(res);
    ASSERT_EQ(file_is_open(file), false);
    file_destroy(file);
};

TEST(test_file, test_file_read) {
    FileState *file = nullptr;
    file_new();
    // manually write to file
    FILE *fd = fopen("/tmp/test_file.txt", "w");
    fwrite("test", sizeof(char), 4, fd);
    fclose(fd);
    char buf[2];
    file_open(file, "/tmp/test_file.txt");
    Result res = file_read(file, buf, 2, 2);
    assert_ok(res);
    ASSERT_STREQ("st", buf);
    res = file_close(file);
    assert_ok(res);
    file_destroy(file);
}

TEST(test_file, test_file_write) {
    FileState *file = nullptr;
    file_new();
    file_open(file, "/tmp/test_file.txt");
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
}