#include "gtest/gtest.h"

extern "C" {
    #include "private/storage/page_manager.h"
    #include "public/util/common.h"
}

// test true
//TEST(test_me, test_page_manager_new) {
//    PageManager *pm;
//    Result res = page_manager_new(&pm);
//    ASSERT_EQ(res, (Result){.resu});
//    ASSERT_EQ(pm->page_size, PAGE_SIZE);
//    ASSERT_EQ(pm->pages_count, 0);
//    ASSERT_EQ(pm->pages, nullptr);
//    ASSERT_EQ(pm->file_manager, nullptr);
//    page_manager_destroy(pm);
//};