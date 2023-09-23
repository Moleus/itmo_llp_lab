#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "util/result.h"
#include "util/helpers.h"

/*
 * здесь должна происходить вся работа с
 * - открытием
 * - закрытием
 * - изменением
 * файлов
 */

typedef struct {
    FILE *file;
} FileState;

Result open_file(FileState *fs, char *filename) {
    ASSERT_ARG_NOT_NULL(fs);
    ASSERT_ARG_NOT_NULL(filename);

    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        file = fopen(filename, "w");
        ASSERT_NOT_NULL(file, FAILED_TO_CREATE_FILE);
        return ERROR("Can't create file");
    }
    fs->file = file;
    return OK;
}

Result close_file(FileState *fs) {
    ASSERT_ARG_NOT_NULL(fs);

    int res = fclose(fs->file);
    if (res != 0) {
        exit_with_msg(FAILED_TO_CLOSE_FILE);
        return ERROR("Failed to close file");
    }
    return OK;
}



// structure which represents file header



