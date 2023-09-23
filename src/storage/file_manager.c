#include "storage/file_manager.h"


Result file_manager_new(FileManager *self) {
    ASSERT_ARG_IS_NULL(self);

    self = malloc(sizeof(FileManager));
    RETURN_IF_NULL(self, "Failed to allocate file manager");
    return OK;
}

Result file_manager_open(FileManager *self, char *filename) {
    ASSERT_ARG_NOT_NULL(self);
    ASSERT_ARG_NOT_NULL(filename);

    FILE *file = fopen(filename, "rb");
    if (file == NULL) {
        file = fopen(filename, "w");
        RETURN_IF_NULL(file, "Can't create file");
    }
    self->file = file;
    return OK;
}