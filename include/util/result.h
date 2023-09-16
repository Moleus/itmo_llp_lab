// define Result struct with OK and ERROR
typedef struct {
    enum {
        RES_OK = 0, RES_ERROR = -1
    } status;
    char *message;
} Result;


// define OK macro
#define OK (Result) {.status = RES_OK, .message = NULL}

// define ERROR macro
#define ERROR(msg) (Result) {.status = RES_ERROR, .message = msg}