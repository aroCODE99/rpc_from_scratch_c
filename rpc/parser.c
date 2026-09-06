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
    Token type;
    Token name;
} Parameter;

// method will have number of args 
typedef struct {
    Token name;
    Parameter *parameters;
    int parameter_count;
    Token return_type;
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

    size_t bytes_read = fread(buff, 1, file_size, file);
    buff[bytes_read] = '\0';
    if (bytes_read > 0) return buff;
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
        log_error("Failed: Did not expected following token");
        exit(1);
    }
    advance_token(parser);
}

void consume_keyword(Parser *parser, char *keyword)
{
    if (!is_keyword(parser->curr_token.start, parser->curr_token.length, keyword)) {
        log_error("Failed due to syntax error: expected %s, got %.*s", keyword, parser->curr_token.length,
                  parser->curr_token.start);
        exit(1);
    }
    advance_token(parser);
}

// this method just checks if the the curr_token is the expected token
int is_current(Parser *parser, TokenType token_type)
{
    return parser->curr_token.type == token_type;
}

// rpc isPrime(int32 number) returns (bool);
// (int32 number)
Parameter *parse_parameter(Parser *parser)
{
    log_info("Parsing the parameter");
    Parameter *parameter = malloc(sizeof(Parameter));
    
    parameter->type =  parser->curr_token;
    consume_token(parser, TOKEN_KEYWORD);
    
    parameter->name = parser->curr_token;
    consume_token(parser, TOKEN_IDENTIFIER);
    
    return parameter;
}

Parameter *parse_parameters(Parser *parser)
{
    log_info("Parsing the parameters");
    consume_token(parser, TOKEN_LPAREN);
    while (!is_current(parser, TOKEN_RPAREN)) {
        Parameter *parameter = parse_parameter(parser);
        // For now just inspect it
        display_token(parameter->type);
        display_token(parameter->name);

        if (is_current(parser, TOKEN_COMMA)) {
            consume_token(parser, TOKEN_COMMA);
        }
    }
    consume_token(parser, TOKEN_RPAREN);
}

Method* parse_method(Parser *parser)
{
    log_info("Parsing the method");
    Method *method = malloc(sizeof(Method));
    consume_keyword(parser, "rpc");

    log_info("copying the method_name");
    method->name = parser->curr_token;
    consume_token(parser, TOKEN_IDENTIFIER);

    method->parameters = parse_parameters(parser);
    
    // returns
    consume_keyword(parser, "returns");

    consume_token(parser, TOKEN_LPAREN);
    method->return_type = parser->curr_token;
    consume_token(parser, TOKEN_KEYWORD);
    consume_token(parser, TOKEN_RPAREN);
    consume_token(parser, TOKEN_SEMICOLON);

    return method;
}

Method *parse_methods(Parser *parser)
{
    log_info("Parsing the methods");
    consume_token(parser, TOKEN_LBRACE);
    while (!is_current(parser, TOKEN_RBRACE)) {
        Method *method = parse_method(parser);
    }
    consume_token(parser, TOKEN_RBRACE);
    return NULL; // temporary
}

// this method is going to parse the whole service
Service* parse_service(Parser* parser)
{
    Service *service = malloc(sizeof(Service));
    consume_keyword(parser, "service"); // now with this method we could consume any keyword
    service->name = service->name;
    consume_token(parser, TOKEN_IDENTIFIER); // we consume the identifier
    service->methods = parse_methods(parser);
    return service;
}

int main()
{
    // so basically what is parser says ?
    // It says: "Give me the next token, and based on what I've already seen, decide what this token means"
    char *buff = read_whole_file_in_buffer(); // reading whole file in the buffer
    Lexer lexer;
    init_lexer(&lexer, buff);
    logger_set_level(LOG_INFO);
    
    Token token;
    Parser parser;
    init_parser(&parser, &lexer); // intializing the parser
    Service *service = parse_service(&parser);
    // now if we could display service;
    
    return 0;
}
