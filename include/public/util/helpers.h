#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <util/log.h>
#include <string.h>

typedef enum {
    NULL_POINTER_IN_ARGS = 0,
    FILE_NOT_FOUND = 2,
    FAILED_TO_CREATE_FILE = 2,
    FAILED_TO_CLOSE_FILE = 3,
    PAGE_ITEM_DATA_IS_NULL = 4,
    NOT_NULL_POINTER_IN_ARGS = 5,
    FAILED_TO_ALLOCATE_MEMORY = 6,
} ErrorCodes;

#define ASSERT_ARG_NOT_NULL(arg) check_arg_null_pointer(arg, #arg, __FILE__, __LINE__);
#define ASSERT_ARG_IS_NULL(arg) check_arg_is_null_pointer(arg, #arg, __FILE__, __LINE__);
#define ASSERT_NOT_NULL(arg, error_code) if (arg == NULL) { exit_with_msg(error_code); }
#define RETURN_IF_NULL(arg, err_msg) if (arg == NULL) { return ERROR(err_msg); }

// enum items descriptions map
char *error_descriptions[] = {
        "NULL pointer in arguments",
        "File not found",
        "Failed to create file",
        "Failed to close file",
        "Page item data is NULL",
        "Argument is NOT empty",
        "Failed to allocate memory",
};

// util method to print error message and description based on error code and exit with code 1
void exit_with_msg(ErrorCodes error_code) {
    if (errno != 0) {
        printf("Error: %s. Caused by %s\n", strerror(errno), error_descriptions[error_code]);
    }
    exit(1);
}

static void exit_with_msg_arg(ErrorCodes error_code, const char *details, const char* file, int line) {
    // TODO: check that it's legal
    char *buf = NULL;
    if (errno != 0) {
        sprintf(buf,"Error: %s. Caused by %s\n", error_descriptions[error_code], strerror(errno));
        log_error(file, line, buf);
    } else {
        sprintf(buf,"Error: %s. Caused by %s\n", error_descriptions[error_code], strerror(errno));
        log_error(file, line, buf);
        printf("Error: %s. Details: %s\n", error_descriptions[error_code], details);
    }

    exit(1);
}

void check_arg_null_pointer(void *arg, const char* arg_name, const char* file, int line) {
    if (arg == NULL) {
        exit_with_msg_arg(NULL_POINTER_IN_ARGS, arg_name, file, line);
    }
}

void check_arg_is_null_pointer(void *arg, const char* arg_name, const char* file, int line) {
    if (arg != NULL) {
        exit_with_msg_arg(NOT_NULL_POINTER_IN_ARGS, arg_name, file, line);
    }
}

