#ifndef LOGGER_H
#define LOGGER_H

#include <stdio.h>

// Define log levels
typedef enum {
    LOG_TRACE,
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARN,
    LOG_ERROR,
    LOG_FATAL
} LogLevel;

// Global configuration setters
void logger_set_level(LogLevel level);
void logger_set_quiet(int quiet);

// The core variadic logging function (called via macros)
void logger_log(LogLevel level, const char *file, int line, const char *fmt, ...);

// Variadic macros to automatically forward __FILE__ and __LINE__
#define log_trace(...) logger_log(LOG_TRACE, __FILE__, __LINE__, __VA_ARGS__)
#define log_debug(...) logger_log(LOG_DEBUG, __FILE__, __LINE__, __VA_ARGS__)
#define log_info(...)  logger_log(LOG_INFO,  __FILE__, __LINE__, __VA_ARGS__)
#define log_warn(...)  logger_log(LOG_WARN,  __FILE__, __LINE__, __VA_ARGS__)
#define log_error(...) logger_log(LOG_ERROR, __FILE__, __LINE__, __VA_ARGS__)
#define log_fatal(...) logger_log(LOG_FATAL, __FILE__, __LINE__, __VA_ARGS__)

#endif // LOGGER_H
