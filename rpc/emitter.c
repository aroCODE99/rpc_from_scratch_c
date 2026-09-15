#include "emitter.h"

int emitter_init(Emitter *emitter)
{
    // i was doing this previously which is wrong becuase we have the emitter pointing to the og object
    // and making this malloc call just says "Forget the old address. Make this pointer point somewhere else."
    // emitter = malloc(sizeof(Emitter));
    if (emitter == NULL) {
        log_error("Failed to allocate the memory");
        return 0;
    }
    emitter->at_line_start = true;
    emitter->indent_str = "    ";
    emitter->indent_level = 0;
    return 1;
}

// sb->length 8
// needed to append str_len 20
static int ensure_resize(StringBuilder *sb, size_t str_length)
{
    size_t required_length = sb->length + str_length + 1; // + 1 for the '\0'
    if (required_length < sb->capacity) {
        return 1; // it is allowed
    }

    // we need to intialize it
    if (sb->capacity == 0) {
        sb->capacity = SB_INIT_CAP;
    }
    
    // this will increase the call to realloc
    // char *ch = realloc(sb->ch, sizeof(char) * require_capacity);
    size_t new_cap = sb->capacity;
    while (new_cap <= required_length) {
        new_cap *= 2;
    }

    // why is it failing here
    char *buff = realloc(sb->buff, new_cap);
    if (buff == NULL) {
        // why is it failing here bc
        log_error("Failed to reallocate");
        return 0;
    }
    sb->buff = buff;
    sb->capacity = new_cap;
    return 1;
}

// TODO: append_first method

int sb_append(StringBuilder *sb, const char *s)
{
    size_t n = strlen(s);
    if (!ensure_resize(sb, n)) {
        return 0;
    }
    memcpy(sb->buff + sb->length, s, n);
    sb->length += n;
    sb->buff[sb->length] = '\0';
    return 1;
}

int sb_append_char(StringBuilder *sb, char c)
{
    if (!ensure_resize(sb, (size_t)1)) {
        return 0;
    }
    sb->buff[sb->length++] = c;
    sb->buff[sb->length] = '\0';
    return 1;
}

void emitter_indent(Emitter *emit) {
    if (emit) emit->indent_level++;
}

void emitter_dedent(Emitter *emit) {
    if (emit && emit->indent_level > 0) {
        emit->indent_level--;
    }
}

int emitter_open_block(Emitter *emitter, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    int needed = vsnprintf(NULL, 0, fmt, args);
    va_end(args);

    if (needed > 0) {
        char *temp = (char*)malloc((size_t)needed + 1);
        if (temp == NULL) {
            log_error("Failed to allocate the Memory");
            return 0;
        }
        va_start(args, fmt);
        vsnprintf(temp, (size_t)needed + 1, fmt, args);
        va_end(args);

        if (!emitter_writeln(emitter, "%s {", temp)) {
            return 0;
        }
        free(temp);
    } else {
        emitter_writeln(emitter, " {");
    }
    emitter_indent(emitter);
    return 1;
}

int emitter_close_block(Emitter *emitter, const char *suffix)
{
    emitter_dedent(emitter);
    if (suffix) {
        emitter_writeln(emitter, "}%s", suffix);
    } else {
        emitter_writeln(emitter, "}");
    }
    return 0;
}

int emitter_write_token(Emitter *emitter, Token token)
{
    if (!emitter_write(emitter, "%.*s", token.length, token.start)) {
        return 0;
    }
    return 1;
}

int emitter_write(Emitter *emitter, const char *fmt, ...)
{
    // 1. Inject indentation if we are starting a clean line
    if (emitter->at_line_start && emitter->indent_level > 0) {
        for (int i = 0; i < emitter->indent_level; i++) {
            sb_append(&emitter->sb, emitter->indent_str);
        }
        emitter->at_line_start = false;
    }

    va_list args;
    va_start(args, fmt);

    int needed = vsnprintf(NULL, 0, fmt, args);
    if (needed < 0) {
        log_error("Failed to determine formatted string size");
        return 0;
    }

    va_end(args);

    if (!ensure_resize(&emitter->sb, needed)) {
        return 0;
    }
    
    va_start(args, fmt);
    needed = vsnprintf(emitter->sb.buff + emitter->sb.length,
                  needed + 1,
                  fmt,
                  args);

    if (needed < 0) {
        log_error("Failed to append");
        return 0;
    }

    emitter->sb.length += needed;
    va_end(args);
    return 1;
}

int emitter_writeln(Emitter *emitter, const char *fmt, ...)
{
    if (fmt && strlen(fmt) > 0) {
        va_list args;
        va_start(args, fmt);

        int needed = vsnprintf(NULL, 0, fmt, args);
        if (needed < 0) {
            log_error("Failed to determine formatted string size");
            return 0;
        }
 
        va_end(args);

        va_start(args, fmt);
        char *temp = malloc(sizeof(char) * needed);
        if (!temp) {
            log_error("Failed to allocate size");
            return 0;
        }
        vsnprintf(temp, (size_t)needed + 1, fmt, args);
        emitter_write(emitter, temp);
        va_end(args);
    }
    sb_append_char(&emitter->sb, '\n');
    emitter->at_line_start = true;
    return 1;
}

void emitter_free(Emitter *emitter)
{
    free(emitter->sb.buff);
    free(emitter);
}
