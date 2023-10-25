#include "private/storage/file_manager.h"
#include "private/storage/file.h"
#include "private/storage/page.h"

FileManager *file_manager_new(void) {
    FileManager *fm = malloc(sizeof(FileManager));
    ASSERT_NOT_NULL(fm, FAILED_TO_ALLOCATE_MEMORY)
    FileState *fs = file_new();
    // fill dest with all fields in structure
    fm->file = fs;
    return fm;
}

void file_manager_destroy(FileManager *self) {
    ASSERT_ARG_NOT_NULL(self)
    ASSERT_ARG_NOT_NULL(self->file)

    Result res = file_close(self->file);
    ABORT_IF_FAIL(res, "Failed to close file")
    file_destroy(self->file);
    free(self);
}

Result file_manager_init(FileManager *self, const char *filename, FileHeaderConstants header_for_new_file) {
    ASSERT_ARG_NOT_NULL(self)
    ASSERT_ARG_NOT_NULL(filename)

    Result res = file_manager_open(self, filename);
    RETURN_IF_FAIL(res, "Failed to open file")

    LOG_INFO("Init file. new: %d. size: %d. Writing header", self->file->is_new, self->file->size);
    if (self->file->size == 1) {
        uint8_t data = 0;
        file_read(self->file, &data, 0, 1);
        LOG_INFO("Small file contents: %hhu", data);
    }
    if (self->file->is_new || self->file->size < sizeof(FileHeader)) {
        self->header.constants = header_for_new_file;
        self->header.dynamic.file_size = sizeof(FileHeader);
        self->header.dynamic.current_free_page = 0;
        self->header.dynamic.page_count = 0;
        res = file_manager_write_header(self);
        LOG_DEBUG("File is new. Writing header", "");
        RETURN_IF_FAIL(res, "Failed to write file header")
    } else {
        res = file_manager_read_header(self);
        RETURN_IF_FAIL(res, "Failed to read file header")
    }
    // check file signature
    if (self->header.constants.signature != header_for_new_file.signature) {
        LOG_WARN("File signature doesn't match. Expected: %x, actual: %x", header_for_new_file.signature,
                 self->header.constants.signature);
        return ERROR("File signature doesn't match");
    }

    LOG_INFO("File manager initialized. Signature: %x, File size: %d. Page size: %d. Page header size: %d", self->header.constants.signature,
             self->header.dynamic.file_size, self->header.constants.page_size, sizeof(PageHeader));

    return OK;
}

Result file_manager_open(FileManager *self, const char *filename) {
    ASSERT_ARG_NOT_NULL(self)
    ASSERT_ARG_NOT_NULL(filename)

    return file_open(self->file, filename);
}

Result file_manager_read_header(FileManager *self) {
    ASSERT_ARG_NOT_NULL(self)

    return file_read(self->file, (void *) &self->header, 0, sizeof(FileHeader));
}

Result file_manager_write_header(FileManager *self) {
    ASSERT_ARG_NOT_NULL(self)

    self->header.dynamic.file_size = sizeof(FileHeader) + self->header.dynamic.page_count * self->header.constants.page_size;
    return file_write(self->file, &self->header.constants, 0, sizeof(FileHeader));
}

Result file_manager_read(FileManager *self, size_t offset, size_t size, void *buffer) {
    ASSERT_ARG_NOT_NULL(self)
    ASSERT_ARG_NOT_NULL(buffer)

    return file_read(self->file, buffer, offset, size);
}

Result file_manager_write(FileManager *self, size_t offset, size_t size, void *buffer) {
    ASSERT_ARG_NOT_NULL(self)
    ASSERT_ARG_NOT_NULL(buffer)

    return file_write(self->file, buffer, offset, size);
}

Result file_manager_get_file_size(FileManager *self, size_t *file_size) {
    ASSERT_ARG_NOT_NULL(self)
    ASSERT_ARG_NOT_NULL(file_size)

    *file_size = self->header.dynamic.file_size;
    return OK;
}
