#include "logger.h"
#include <stdarg.h>
#include <time.h>

static LogLevel current_level = LOG_TRACE;
static int is_quiet = 0;

// Text representations for log levels
// LEFTEST includes every thing till that level
static const char *level_strings[] = {
    "TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL"
};

// ANSI Color escapes for terminal formatting
static const char *level_colors[] = {
    "\x1b[94m", "\x1b[36m", "\x1b[32m", "\x1b[33m", "\x1b[31m", "\x1b[35m"
};

void logger_set_level(LogLevel level) {
    current_level = level;
}

void logger_set_quiet(int quiet) {
    is_quiet = quiet;
}

void logger_log(LogLevel level, const char *file, int line, const char *fmt, ...) {
    // Filter out messages below the active log level
    if (level < current_level || is_quiet) {
        return;
    }

    // Capture the current system timestamp
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    char time_buf[16];
    strftime(time_buf, sizeof(time_buf), "%H:%M:%S", tm_info);

    // va is just the variable args that we are getting can be used for the formated strings
    
    // Format out to standard error (stderr)
    va_list args;
    va_start(args, fmt);

    // Print standard metadata header: [Time] [Level] [File:Line]
    fprintf(stderr, "%s %s%-5s\x1b[0m \x1b[90m%s:%d:\x1b[0m ",
            time_buf, level_colors[level], level_strings[level], file, line);
    
    // Print the user's custom message
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n"); // after user's message we need a newline
    
    va_end(args);
    fflush(stderr);
}
