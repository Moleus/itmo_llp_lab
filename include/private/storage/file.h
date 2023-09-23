#pragma once

#include <stdbool.h>
#include "public/storage/file.h"

struct FileState {
    FILE *file;
    bool is_open;
};
