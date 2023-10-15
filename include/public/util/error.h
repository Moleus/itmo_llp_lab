#pragma once

#include <stdlib.h>
#include <string.h>

enum ErrorCodes {
    NULL_POINTER_IN_ARGS = 0,
    FILE_NOT_FOUND = 2,
    FAILED_TO_CREATE_FILE = 2,
    FAILED_TO_CLOSE_FILE = 3,
    PAGE_ITEM_DATA_IS_NULL = 4,
    NOT_NULL_POINTER_IN_ARGS = 5,
    FAILED_TO_ALLOCATE_MEMORY = 6,
    INTERNAL_LIB_ERROR = 7,
};

// enum items descriptions map
static const char *error_descriptions[] = {
        "NULL pointer in arguments",
        "File not found",
        "Failed to create file",
        "Failed to close file",
        "Page item data is NULL",
        "Argument is NOT empty",
        "Failed to allocate memory",
        "Internal library error",
};

#define ASSERT_ARG_NOT_NULL(arg) \
    if (arg == NULL) { \
        LOG_ERR("%s. argument: %s", error_descriptions[NULL_POINTER_IN_ARGS], #arg); \
        exit(1);                     \
    }

#define ASSERT_ARG_IS_NULL(arg) \
    if (arg != NULL) { \
        LOG_ERR("%s. argument: %s", error_descriptions[NOT_NULL_POINTER_IN_ARGS], #arg); \
        exit(1);                     \
    }

#define ASSERT_NOT_NULL(arg, error_code) \
    if ((arg) == NULL) { \
        LOG_ERR("%s", error_descriptions[error_code]); \
        exit(1);                     \
    }

#define ABORT_EXIT(error_code, err_msg) \
    LOG_ERR("Error: %s. Details: %s", error_descriptions[error_code], err_msg); \
    exit(1);

#define RETURN_IF_NULL(arg, err_msg) if ((arg) == NULL) { return ERROR(err_msg); }
