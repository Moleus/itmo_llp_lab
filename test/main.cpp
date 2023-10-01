#include <gtest/gtest.h>

extern "C" {
#include "public/util/log.h"
}

unsigned char log_level = DEBUG;

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}