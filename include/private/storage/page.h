#pragma once

#include "public/storage/page.h"

struct Page {
    PageHeader page_header;
    // offset from the start of the file where page starts
    int32_t offset;
    int32_t page_id;
};