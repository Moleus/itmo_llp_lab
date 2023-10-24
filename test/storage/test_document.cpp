#include "gtest/gtest.h"
#include <cstdint>
#include "common.h"

extern "C" {
#include "private/storage/page_manager.h"
#include "public/util/common.h"
}

#define PAGE_SIZE 80
#define SIGNATURE 0x12345678

TEST(document, create_multiple_nodes_delete_one) {

}