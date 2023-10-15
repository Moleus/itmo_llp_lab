#pragma once

#include <stdlib.h>
#include <string.h>

enum ErrorCodes {
    NULL_POINTER_IN_ARGS = 0,
    NOT_NULL_POINTER_IN_ARGS = 1,
    FAILED_TO_ALLOCATE_MEMORY = 2,
    INTERNAL_LIB_ERROR = 3,
};

// enum items descriptions map
static const char *error_descriptions[] = {
        "NULL pointer in arguments",
        "Argument is NOT empty",
        "Failed to allocate memory",
        "Internal library error",
};

#define ASSERT_ARG_NOT_NULL(arg) \
    if ((arg) == NULL) { \
        LOG_ERR("%s. argument: %s", error_descriptions[NULL_POINTER_IN_ARGS], #arg); \
        exit(1);                     \
    }

#define ASSERT_ARG_IS_NULL(arg) \
    if ((arg) != NULL) { \
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
