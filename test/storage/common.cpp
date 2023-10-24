#include <cstdlib>
#include "common.h"
#include <cstdio>
#include "private/storage/file.h"

void remove_file() {
    bool file_exists = access(FILE_PATH, F_OK) == 0;
    if (file_exists && remove(FILE_PATH) == -1) {
        printf("Failed to remove file %s\n", FILE_PATH);
        exit(1);
    }
}

