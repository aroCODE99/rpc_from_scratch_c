#include "lexer.h"
#include "logger.h"
#include "vector.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// i think this will require the vector implementation
typedef struct {
    Lexer *lexer;
    Token curr_token;
} Parser;

typedef struct {
    Token type;
    Token name;
} Parameter;

// method will have number of args
// now this is very similar to the
// parameters
//    ├── Parameter { type = "int",    name = "id" }
//    └── Parameter { type = "string", name = "name" }
typedef struct {
    Token name;
    vector parameters;
    Token return_type;
} Method;

// so there is going to be Service
// which will have multiple methods
// above diagram goes for methods also
typedef struct {
    Token name;
    vector methods;
} Service;

void display_service(Service *service)
{
    // just displaying the name
    display_token(service->name);
    // printing methods
    for (int i = 0; i < service->methods.total; ++i) {
        Method *curr_method = service->methods.items[i];
        display_token(curr_method->name);
        // displaying the parameter
        for (int j = 0; j < curr_method->parameters.total; ++j) {
            Parameter *curr_parameter = curr_method->parameters.items[j];
            display_token(curr_parameter->name);
            display_token(curr_parameter->type);
        }
        display_token(curr_method->return_type);
    }
}

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

int is_current(Parser *parser, TokenType token_type)
{
    return parser->curr_token.type == token_type;
}

// this will give use a one pair of parameter
// (int32 number, int32 another_number) => int32 number
Parameter *parse_parameter(Parser *parser)
{
    log_info("Parsing the parameter");
    Parameter *parameter = malloc(sizeof(Parameter));
    if (parameter == NULL) {
        log_error("Failed to allocate the memory");
        exit(1);
    }

    parameter->type =  parser->curr_token;
    consume_token(parser, TOKEN_KEYWORD);
    
    parameter->name = parser->curr_token;
    consume_token(parser, TOKEN_IDENTIFIER);
    
    return parameter;
}

void parse_parameters(Parser *parser, vector *parameters)
{
    log_info("Parsing the parameters");
    consume_token(parser, TOKEN_LPAREN);

    // adding the parameter to the parameters vector
    while (!is_current(parser, TOKEN_RPAREN)) {
        Parameter *parameter = parse_parameter(parser);
        vector_add(parameters, parameter);

        // consuming the ','
        if (is_current(parser, TOKEN_COMMA)) {
            consume_token(parser, TOKEN_COMMA);
        }
    }
    consume_token(parser, TOKEN_RPAREN);
}

Method* parse_method(Parser *parser)
{
    // i really don't know what i am doing at this point
    log_info("Parsing the method");
    // initializing the method struct
    Method *method = malloc(sizeof(Method));
    if (method == NULL) {
        log_error("Failed to allocate the memory");
        exit(1);
    }
    vector_init(&method->parameters);
    
    consume_keyword(parser, "rpc");

    log_info("copying the method_name");
    method->name = parser->curr_token;
    consume_token(parser, TOKEN_IDENTIFIER);

    parse_parameters(parser, &method->parameters);
    
    // returns
    consume_keyword(parser, "returns");

    consume_token(parser, TOKEN_LPAREN);
    method->return_type = parser->curr_token;
    consume_token(parser, TOKEN_KEYWORD);
    consume_token(parser, TOKEN_RPAREN);
    consume_token(parser, TOKEN_SEMICOLON);

    return method;
}

// so this method is not returning anything
// should this be taking the Service* as a parameter
// now i am really confuse here like how should i be using this
// like Method is list Method[] of methods
void parse_methods(Parser *parser, vector *methods)
{
    log_info("Parsing the methods");
    consume_token(parser, TOKEN_LBRACE);

    // we are adding the method to the methods vector
    while (!is_current(parser, TOKEN_RBRACE)) {
        Method *method = parse_method(parser);
        vector_add(methods, method);
    }
    consume_token(parser, TOKEN_RBRACE);
}

// this method is going to parse the whole service
Service* parse_service(Parser* parser)
{
    // initializing service struct
    log_info("parsing service");
    Service *service = malloc(sizeof(Service));

    // I was doing this first
    // VECTOR_INIT(vec);
    // service->methods = &vec;
    // but the vec is the local variable and it is gone with this function call
    // method
    // parse_method()
    // │
    // ├── vec        <-- local variable
    // │
    // └── return method
    //        │
    //        └── parameters ---> vec

    // allocating the size for the methods
    // so (vector *pointer) is very diff from the 
    // vector *parameters
    //        |
    //        |-----> ????
    // vector vec;
    // this actually has everything in the struct
    // vec
    // +----------------+
    // | items          |
    // | cap            |
    // | total          |
    // +----------------+


    // Instead of using the vector * using vector directly so we don't manage two heaps
    // RULE: If the parent logically owns exactly one object and that object has the same lifetime as the parent,
    // embedding the object is usually simpler.    vector_init(&service->methods);
    
    consume_keyword(parser, "service"); // now with this method we could consume any keyword
    // what the f am i doing here
    service->name = parser->curr_token; // storing the identifier token_name
    consume_token(parser, TOKEN_IDENTIFIER); // we consume the identifier
    parse_methods(parser, &service->methods);
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
    display_service(service);
    return 0;
}
