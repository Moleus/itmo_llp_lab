#include "log.h"

// define Result struct with OK and ERROR
typedef struct {
    enum {
        RES_OK = 0, RES_ERROR = -1
    } status;
    const char *message;
} Result;


// define OK macro
#define OK (Result) {.status = RES_OK, .message = NULL}

// define ERROR macro
#define ERROR(msg) (Result) {.status = RES_ERROR, .message = msg}


bool result_is_fail__(Result result) {
    return result.status == RES_ERROR;
}

// requires PACKAGE_NAME to be in the function context
#define RETURN_IF_FAIL(result, return_error_msg) \
    if (result_is_fail__(result)) {      \
        log_error(__FILE__, __LINE__, result.message);       \
        return ERROR(return_error_msg);          \
    }