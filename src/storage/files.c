#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>
#include "util/result.h"

/*
 * здесь должна происходить вся работа с
 * - открытием
 * - закрытием
 * - изменением
 * файлов
 */

// Check argument it not null macro
#define CHECK_ARG_NOT_NULL(arg) if (arg == NULL) { exit_with_msg(NULL_POINTER_IN_ARGS); }

// macro which checks for null and if it's null then call error with provided error code
#define ASSERT_NOT_NULL(arg, error_code) if (arg == NULL) { exit_with_msg(error_code); }

typedef enum {
    NULL_POINTER_IN_ARGS = 0,
    FILE_NOT_FOUND = 2,
    FAILED_TO_CREATE_FILE = 2,
    FAILED_TO_CLOSE_FILE = 3,
} ErrorCodes;

// enum items descriptions map
char *error_descriptions[] = {
        "NULL pointer in arguments",
        "File not found",
        "Failed to create file",
        "Failed to close file",
};

// util method to print error message and description based on error code and exit with code 1
void exit_with_msg(ErrorCodes error_code) {
    if (errno != 0) {
        printf("Error: %s. Caused by %s\n", strerror(errno), error_descriptions[error_code]);
    }
    exit(1);
}

typedef struct {
    FILE *file;
} FileState;

Result open_file(FileState *fs, char *filename) {
    CHECK_ARG_NOT_NULL(fs);
    CHECK_ARG_NOT_NULL(filename);

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
    CHECK_ARG_NOT_NULL(fs);

    int res = fclose(fs->file);
    if (res != 0) {
        exit_with_msg(FAILED_TO_CLOSE_FILE);
        return ERROR("Failed to close file");
    }
    return OK;
}



// structure which represents file header



