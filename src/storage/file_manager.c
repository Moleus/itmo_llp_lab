#include "private/storage/file_manager.h"

Result file_manager_new(FileManager **dest) {
    ASSERT_ARG_IS_NULL(*dest);

    *dest = malloc(sizeof(FileManager));
    RETURN_IF_NULL(dest, "Failed to allocate file manager");
    FileState *fs = NULL;
    Result res = file_new(&fs);
    // TODO: free self memory if fail.
    // Implement auto clean macro on fail
    RETURN_IF_FAIL(res, "Failed to allocate FileState");
    // fill dest with all fields in structure
    (*dest)->file = fs;
    (*dest)->header = (FileHeader) {
        .signature = 0,
        .size = 0,
        .first_free_page_id = 0,
        .last_free_page_id = 0,
        .page_count = 0,
        .records_count = 0
    };

    return OK;
}

Result file_manager_destroy(FileManager *self) {
    ASSERT_ARG_NOT_NULL(self);
    ASSERT_ARG_NOT_NULL(self->file);

    Result res = file_destroy(self->file);
    RETURN_IF_FAIL(res, "Failed to destroy file");
    free(self);
    return OK;
}

Result file_manager_open(FileManager *self, char *filename) {
    ASSERT_ARG_NOT_NULL(self);
    ASSERT_ARG_NOT_NULL(filename);

    return file_open(self->file, filename);
}

Result file_manager_read_header(FileManager *self, FileHeader *header) {
    ASSERT_ARG_NOT_NULL(self);
    ASSERT_ARG_NOT_NULL(header);

    return file_read(self->file, header, 0, sizeof(FileHeader));
}

Result file_manager_write_header(FileManager *self, FileHeader *header) {
    ASSERT_ARG_NOT_NULL(self);
    ASSERT_ARG_NOT_NULL(header);

    return file_write(self->file, header, 0, sizeof(FileHeader));
}

Result file_manager_read(FileManager *self, size_t offset, size_t size, void *buffer) {
    ASSERT_ARG_NOT_NULL(self);
    ASSERT_ARG_NOT_NULL(buffer);

    return file_read(self->file, buffer, offset, size);
}

Result file_manager_write(FileManager *self, size_t offset, size_t size, void *buffer) {
    ASSERT_ARG_NOT_NULL(self);
    ASSERT_ARG_NOT_NULL(buffer);

    return file_write(self->file, buffer, offset, size);
}

Result file_manager_close(FileManager *self) {
    ASSERT_ARG_NOT_NULL(self);

    return file_close(self->file);
}