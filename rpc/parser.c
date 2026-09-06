#include "lexer.h"
#include "logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    Lexer *lexer;
    Token curr_token;
} Parser;

typedef struct {
    char *name;
    char *type;
} Parameter;

// method will have number of args 
typedef struct {
    char *name;
    Parameter *parameters;
    int parameter_count;
    char *return_type;
} Method;

// so there is going to be Service
// which will have multiple methods
typedef struct {
    char *name;
    Method *methods;
    int method_count;
} Service;

char *read_whole_file_in_buffer()
{
    char *path = "./test.rpc";
    log_info("reading the whole %s into the buffer", path);

    FILE *file = fopen(path, "r");
    if (file == NULL) {
        perror("Error");
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    int file_size = ftell(file);
    rewind(file);

    char *buff = malloc(file_size * sizeof(char));
    if (buff == NULL) {
        perror("Error");
        return NULL;
    }

    size_t bytesRead = fread(buff, 1, file_size, file);
    if (bytesRead > 0) return buff;
    return NULL;
}

void init_parser(Parser *parser, Lexer *lexer)
{
    parser->lexer = lexer;
    parser->curr_token = get_next_token(lexer);
}

void advance_token(Parser *parser)
{
    parser->curr_token = get_next_token(parser->lexer);
}

void consume_token(Parser *parser, TokenType expected)
{
    if (parser->curr_token.type != expected) {
        fprintf(stderr, "Syntax error on line %d\n", parser->curr_token.line);
        exit(1);
    }
    advance_token(parser);
}

void consume_keyword(Parser *parser, char *keyword)
{
    if (!is_keyword(parser->curr_token.start, parser->curr_token.length, keyword)) {
        log_error("Failed due to syntax error");
        exit(1);
    }
    advance_token(parser);
}

// this method just checks if the the curr_token is the expected token
int is_current(Parser *parser, TokenType token_type)
{
    return parser->curr_token.type == token_type;
}

Method* parse_method(Parser *parser)
{
    consume_keyword(parser, "rpc");
}

Method* parse_methods(Parser *parser)
{
    consume_token(parser, TOKEN_LBRACE); // the first thing is the '{'
    // why is this not working as expected
    while (!is_current(parser, TOKEN_RBRACE)) {
        Method *method = parse_method(parser);
        advance_token(parser);
    }
}

// this method is going to parse the whole service
Service* parse_service(Parser* parser)
{
    Service *service = malloc(sizeof(Service));
    consume_keyword(parser, "service"); // now with this method we could consume any keyword
    service->name = service->name;
    consume_token(parser, TOKEN_IDENTIFIER); // we consume the identifier
    Method *methods = parse_methods(parser);
    return service;
}

int main()
{
    // so basically what is parser says ?
    // It says: "Give me the next token, and based on what I've already seen, decide what this token means"
    char *buff = read_whole_file_in_buffer(); // reading whole file in the buffer
    Lexer lexer;
    init_lexer(&lexer, buff);
    
    Token token;
    Parser parser;
    init_parser(&parser, &lexer);
    Token curr;
    while ((curr = get_next_token(&lexer)).type != TOKEN_EOF) {
        display_token(curr);
    }
    
    return 0;
}
