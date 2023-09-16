// File header structure
#include <stdlib.h>
#include "util/result.h"
#include "file_manager/file_manager.h"

typedef struct {
    size_t size;
    size_t first_free_page_id;
    size_t last_free_page_id;
    size_t header_size;
    size_t page_header_size;
    size_t page_count;
    size_t records_count;
} FileHeader;

// read file header from file
Result file_header_read(FileManager *file_manager, FileHeader *header);

// write file header to file
Result file_header_write(FileManager *file_manager, FileHeader *header);
