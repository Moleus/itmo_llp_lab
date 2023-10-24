#pragma once

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdarg.h>

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

static void LOG(LogLevel level, const char *file, int line, const char* format, ...) {
    va_list args;
    va_start (args, format);
    if (log_level > level) {
        return;
    }
    FILE *out = stdout;
    if (level >= WARN) {
        out = stderr;
    }
    fprintf(out, "[%s] [%s:%d]: ", log_level_names[level], file, line);
    vfprintf(out, format, args);
    fputc('\n', out);
    fflush(out);
    va_end (args);
}

#define LOG_DEBUG(format, ...) LOG(DEBUG, __FILE__, __LINE__, format, __VA_ARGS__)

#define LOG_INFO(format, ...) LOG(INFO, __FILE__, __LINE__, format, __VA_ARGS__)

#define LOG_WARN(format, ...) LOG(WARN, __FILE__, __LINE__, format, __VA_ARGS__)

#define LOG_ERR(format, ...) LOG(ERROR, __FILE__, __LINE__, format, __VA_ARGS__)
