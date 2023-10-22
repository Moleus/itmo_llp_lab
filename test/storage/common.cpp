#include <cstdlib>
#include "common.h"
#include <cstdio>

void remove_file() {
    if (remove(FILE_PATH) == -1) {
        printf("Failed to remove file %s\n", FILE_PATH);
        exit(1);
    }
}

