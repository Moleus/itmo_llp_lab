#include "private/storage/file.h"
#include <assert.h>

FileState * file_new() {
    FileState *fs = malloc(sizeof(FileState));
    ASSERT_NOT_NULL(fs, FAILED_TO_ALLOCATE_MEMORY);
    fs->is_open = false;
    fs->is_new = false;
    return fs;
}

Result file_destroy(FileState *fs) {
    ASSERT_ARG_NOT_NULL(fs);
    assert(fs->is_open == false);

    free(fs);
    return OK;
}

Result file_open(FileState *fs, const char *filename) {
    ASSERT_ARG_NOT_NULL(fs);
    ASSERT_ARG_NOT_NULL(filename);
    assert(fs->is_open == false);

    // try to read the file
    FILE *file = fopen(filename, "r+b");
    if (file == NULL) {
        // if it doesn't exist - set flag and create new file
        file = fopen(filename, "a+b");
        fs->is_new = true;
        RETURN_IF_NULL(file, "Can't open file");
    }
    fs->file = file;
    fs->is_open = true;
    file_get_file_size(fs, &fs->size);
    return OK;
}

Result file_close(FileState *fs) {
    ASSERT_ARG_NOT_NULL(fs);
    assert(fs->is_open == true);

    int res = fclose(fs->file);
    if (res != 0) {
        return ERROR("Failed to close file");
    }
    fs->is_open = false;
    return OK;
}

Result file_write(FileState *fs, void *data, size_t offset, size_t size) {
    ASSERT_ARG_NOT_NULL(fs);
    ASSERT_ARG_NOT_NULL(data);
    assert(fs->is_open == true);

    // TODO: check the cast to long
    int res = fseek(fs->file, (long) offset, SEEK_SET);
    if (res != 0) {
        return ERROR("Failed to set file offset");
    }
    // TODO: can we write more than 1 byte? (size > 1)
    size_t written = fwrite(data, sizeof(char), size, fs->file);
    if (written != size) {
        return ERROR("Failed to write to file");
    }
    return OK;
}

Result file_read(FileState *fs, void *data, size_t offset, size_t size) {
    ASSERT_ARG_NOT_NULL(fs);
    ASSERT_ARG_NOT_NULL(data);
    assert(fs->is_open == true);

    int res = fseek(fs->file, (long) offset, SEEK_SET);
    if (res != 0) {
        return ERROR("Failed to set file offset");
    }
    size_t read = fread(data, sizeof(char), size, fs->file);
    if (read != size) {
        return ERROR("Failed to read from file");
    }
    return OK;
}

bool file_is_open(FileState *fs) {
    ASSERT_ARG_NOT_NULL(fs);

    return fs->is_open;
}

Result file_get_file_size(FileState *fs, size_t *file_size) {
    ASSERT_ARG_NOT_NULL(fs);
    ASSERT_ARG_NOT_NULL(file_size);

    int res = fseek(fs->file, 0L, SEEK_END);
    if (res != 0) {
        return ERROR("Failed to set file offset");
    }
    *file_size = ftell(fs->file);
    return OK;
}