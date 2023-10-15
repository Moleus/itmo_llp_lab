#include "private/storage/file_manager.h"

FileManager * file_manager_new() {
    FileManager *fm = malloc(sizeof(FileManager));
    ASSERT_NOT_NULL(fm, FAILED_TO_ALLOCATE_MEMORY)
    FileState *fs = file_new();
    // TODO: free self memory if fail.
    // fill dest with all fields in structure
    fm->file = fs;
    return fm;
}

void file_manager_destroy(FileManager *self) {
    ASSERT_ARG_NOT_NULL(self);
    ASSERT_ARG_NOT_NULL(self->file);

    file_destroy(self->file);
    free(self);
}

Result file_manager_init(FileManager *self, const char *filename, FileHeaderConstants header_for_new_file) {
    ASSERT_ARG_NOT_NULL(self);
    ASSERT_ARG_NOT_NULL(filename);

    Result res = file_manager_open(self, filename);
    RETURN_IF_FAIL(res, "Failed to open file")

    if (self->file->is_new || self->file->size == 0) {
        self->header.constants = header_for_new_file;
        self->header.dynamic.file_size = 0;
        self->header.dynamic.current_free_page = 0;
        self->header.dynamic.page_count = 0;
        res = file_manager_write_header(self);
        RETURN_IF_FAIL(res, "Failed to write file header")
    } else {
        res = file_manager_read_header(self);
        RETURN_IF_FAIL(res, "Failed to read file header")
    }

    return OK;
}

Result file_manager_open(FileManager *self, const char *filename) {
    ASSERT_ARG_NOT_NULL(self);
    ASSERT_ARG_NOT_NULL(filename);

    return file_open(self->file, filename);
}

Result file_manager_read_header(FileManager *self) {
    ASSERT_ARG_NOT_NULL(self);

    FileHeader header = self->header;
    return file_read(self->file, (void *) &header, 0, sizeof(FileHeader));
}

Result file_manager_write_header(FileManager *self) {
    ASSERT_ARG_NOT_NULL(self);

    return file_write(self->file, &self->header, 0, sizeof(FileHeader));
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

Result file_manager_get_file_size(FileManager *self, size_t *file_size) {
    ASSERT_ARG_NOT_NULL(self);
    ASSERT_ARG_NOT_NULL(file_size);

    *file_size = self->header.dynamic.file_size;
    return OK;
}