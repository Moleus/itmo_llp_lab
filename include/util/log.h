#include <stdio.h>

typedef enum {
    DEBUG = 0,
    INFO = 1,
    WARN = 2,
    ERROR = 3,
} LogLevel;

extern unsigned char log_level;

char *log_level_names[] = {
        "DEBUG",
        "INFO",
        "WARN",
        "ERROR",
};

static void log(LogLevel level, const char *file, int line, const char *message) {
    if (log_level > level) {
        return;
    }
    FILE *out = stdout;
    if (log_level >= WARN) {
        out = stderr;
    }
    fprintf(out, "[%s] [%s:%d]: %s\n", log_level_names[level], file, line, message);
}

void log_debug(const char *file, int line, const char *message) {
    log(DEBUG, file, line, message);
}

void log_info(const char *file, int line, const char *message) {
    log(INFO, file, line, message);
}

void log_warn(const char *file, int line, const char *message) {
    log(WARN, file, line, message);
}

void log_error(const char *file, int line, const char *message) {
    log(ERROR, file, line, message);
}
