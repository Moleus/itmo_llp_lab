#pragma once

#include "util/result.h"
#include <stdio.h>
#include <stdlib.h>

typedef struct {
    int64_t signature;
    size_t size;
    size_t first_free_page_id;
    size_t last_free_page_id;
    size_t page_count;
    size_t records_count;
} FileHeader;

// file object which stores pointer to file
typedef struct {
    FILE *file;
    FileHeader header;
} FileManager;

Result file_manager_new(FileManager *self);

Result file_manager_open(FileManager *self, char *filename);

// read file header from file
Result file_manager_read_header(FileManager *file_manager, FileHeader *header);

// write file header to file
Result file_manager_write_header(FileManager *file_manager, FileHeader *header);

Result file_manager_read(FileManager *self, size_t offset, size_t size, void *buffer);
Result file_manager_write(FileManager *self, size_t offset, size_t size, void *buffer);

Result file_manager_close(FileManager *self);

Result file_manager_destroy(FileManager *self);