#ifndef PARSER_H
#define PARSER_H

#include "ast.h"
#include "lexer.h"
#include "logger.h"

#define VECTOR_IMPLEMENTATION
#include "vector.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// i think this will require the vector implementation
typedef struct {
    Lexer *lexer;
    Token curr_token;
} Parser;

const char *token_type_name(TokenType type);
void syntax_error(Parser *parser, const char *message);
void init_parser(Parser *parser, Lexer *lexer);
void advance_token(Parser *parser);
void consume_token(Parser *parser, TokenType expected);
void consume_keyword(Parser *parser, const char *keyword);
int is_current(Parser *parser, TokenType token_type);
Parameter *parse_parameter(Parser *parser);
void parse_parameters(Parser *parser, vector *parameters);
Method* parse_method(Parser *parser);
void parse_methods(Parser *parser, vector *methods);
Service* parse_service(Parser* parser);

#endif

