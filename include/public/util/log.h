#pragma once

#include <stdio.h>
#include <errno.h>
#include <string.h>

typedef enum {
    DEBUG = 0,
    INFO = 1,
    WARN = 2,
    ERROR = 3,
} LogLevel;

extern unsigned char log_level;

static const char *log_level_names[] = {
        "DEBUG",
        "INFO",
        "WARN",
        "ERROR",
};

static void LOG(LogLevel level, const char *file, int line, const char *message) {
    if (log_level > level) {
        return;
    }
    FILE *out = stdout;
    if (level >= WARN) {
        out = stderr;
    }
    if (errno != 0) {
        fprintf(out, "[%s] [%s:%d]: %s. (errno: %s)\n", log_level_names[level], file, line, message, strerror(errno));
    } else {
        fprintf(out, "[%s] [%s:%d]: %s\n", log_level_names[level], file, line, message);
    }
}

static void log_debug(const char *file, int line, const char *message) {
    LOG(DEBUG, file, line, message);
}

static void log_info(const char *file, int line, const char *message) {
    LOG(INFO, file, line, message);
}

static void log_warn(const char *file, int line, const char *message) {
    LOG(WARN, file, line, message);
}

static void log_error(const char *file, int line, const char *message) {
    LOG(ERROR, file, line, message);
}
