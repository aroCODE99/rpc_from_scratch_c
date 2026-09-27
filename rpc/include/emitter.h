#ifndef EMITTER_H
#define EMITTER_H

#include "lexer.h"
#include "logger.h"
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdbool.h>

#define SB_INIT_CAP 8

typedef struct {
    char *buff;
    size_t capacity;
    size_t length;
} StringBuilder;

typedef struct {
    StringBuilder sb;
    int indent_level;
    const char *indent_str;
    bool at_line_start;
} Emitter;

// sb public api's
int emitter_init(Emitter *);
int emitter_write_token(Emitter *, Token);
int emitter_write(Emitter *, const char *fmt, ...);
int emitter_writeln(Emitter *, const char *fmt, ...);
int emitter_open_block(Emitter *, const char *, ...);
int emitter_close_block(Emitter *, const char *);
void emitter_free(Emitter *);

#endif
