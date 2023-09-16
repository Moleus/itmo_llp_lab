#include "util/result.h"
#include <stdio.h>

// file object which stores pointer to file
typedef struct {
    FILE *file;
} FileManager;

Result file_manager_new(FileManager *self);

Result file_manager_open(FileManager *self, char *filename);

Result file_manager_read(FileManager *self, size_t offset, size_t size, char *buffer);
Result file_manager_write(FileManager *self, size_t offset, size_t size, char *buffer);

Result file_manager_close(FileManager *self);

Result file_manager_destroy(FileManager *self);