#ifndef UTILITY_H
#define UTILITY_H

#include "logger.h"
#include "ast.h"
#include "lexer.h"
#include "parser.h"
#include "emitter.h"
#include "code_gen.h"
#include "code_gen_client.h"
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

typedef struct {
    const char *program_name;
    const char *input;
    const char *output_dir;
    bool client;
    bool server;
    bool mode_specified;
} Options;

char *read_whole_file_in_buffer(const char *);
int parse_args(int , char **, Options *);
Service *compile_file(const char *);
int write_output_file(const char *, const char *, const char *);
int generate_files(Service *, Options *);

#endif
