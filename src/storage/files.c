#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "util/result.h"
#include "util/helpers.h"

typedef struct {
    FILE *file;
} FileState;

Result file_open(FileState *fs, char *filename) {
    ASSERT_ARG_NOT_NULL(fs);
    ASSERT_ARG_NOT_NULL(filename);

    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        file = fopen(filename, "w");
        RETURN_IF_NULL(file, "Can't create file");
    }
    fs->file = file;
    return OK;
}

Result file_close(FileState *fs) {
    ASSERT_ARG_NOT_NULL(fs);

    int res = fclose(fs->file);
    if (res != 0) {
        return ERROR("Failed to close file");
    }
    return OK;
}

Result file_write(FileState *fs, void *data, size_t offset, size_t size) {
    ASSERT_ARG_NOT_NULL(fs);
    ASSERT_ARG_NOT_NULL(data);

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