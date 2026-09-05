#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

typedef enum {
    TOKEN_KEYWORD,
    TOKEN_IDENTIFIER,
    TOKEN_NUMBER,
    TOKEN_OPERATOR,
    TOKEN_EOF,
    TOKEN_ERROR,
    
    // New Brackets and Braces
    TOKEN_LPAREN,   // (
    TOKEN_RPAREN,   // )
    TOKEN_LBRACE,   // {
    TOKEN_RBRACE,   // }
    TOKEN_LBRACKET, // [
    TOKEN_RBRACKET, // ]

} TokenType;

// main token object
typedef struct {
    TokenType type;
    const char* start;
    int length;       
    int line;         
} Token;

typedef struct {
    const char *source;
    char curr_char;
    int index;
    int line;
} Lexer;

// helper methods
void init_lexer(Lexer *lexer, const char *source)
{
    printf("initializing the lexer\n");
    lexer->source = source;
    lexer->curr_char = source[0];
    lexer->index = 0;
    lexer->line = 1;
}

void advance(Lexer *lexer)
{
    if (lexer->curr_char != '\0') {
        lexer->index += 1;
        lexer->curr_char = lexer->source[lexer->index];
    }
}

// show me the next_char
char peek(Lexer *lexer)
{
    if (lexer->curr_char == '\0') return '\0';
    return lexer->source[lexer->index + 1];
}

// handling the non-token entities
void skip_whitespace_and_comments(Lexer *lexer)
{
    while (lexer->curr_char != '\0') {
        // space, tab and carriage return
        if (lexer->curr_char == ' ' || lexer->curr_char == '\t' || lexer->curr_char == '\r') {
            // we just skip it
            advance(lexer);
        } else if (lexer->curr_char == '\n') {
            lexer->line += 1; // newLine
            advance(lexer);
        } else if (lexer->curr_char == '/' && peek(lexer) == '/') {
            // meaning that this line is comment and we need to skipp the whole line
            while (lexer->curr_char != '\n' && lexer->curr_char != '\0') {
                advance(lexer); // skipp all the char's in this line
            }
        } else {
            break;
        }
    }
}

//Match keywords against a set of predefined words
int is_keyword(const char *start, int length, const char *keyword)
{
    return length == (int)strlen(keyword) && strncmp(start, keyword, length) == 0;
}

// now how does this works let's see first in action with the examples
// int_sfs -> this is the identifier
// int identifier -> int is the keyword
Token read_identifier_or_keyword(Lexer *lexer)
{
    const char *start = &lexer->source[lexer->index];
    int line = lexer->line;

    // identifier could also have the numbers and '_'
    while (isalnum(lexer->curr_char) || lexer->curr_char == '_') {
        advance(lexer);
    }
    // now we got the token
    int length = (int)(&lexer->source[lexer->index] - start);

    // now checking if it is the identifier or keyword
    if (is_keyword(start, length, "int") || is_keyword(start, length, "return") || 
        is_keyword(start, length, "if")  || is_keyword(start, length, "else")) {
        return (Token){TOKEN_KEYWORD, start, length, line};
    }

    // or else it is a identifier
    return (Token){TOKEN_IDENTIFIER, start, length, line};
}

// now what is the goal of this method
Token read_number(Lexer *lexer)
{
    const char *start = &lexer->source[lexer->index];
    int line = lexer->line;

    while (isdigit(lexer->curr_char)) {
        advance(lexer);
    }
    int length = (int)(&lexer->source[lexer->index] - start);
    return (Token) {TOKEN_NUMBER, start, length, line};
}

Token get_single_char_token(Lexer *lexer, char *start, int line, TokenType token_type)
{
    advance(lexer);
    int length = (int)(&lexer->source[lexer->index] - start);
    return (Token) {token_type, start, length, line};
}

Token get_next_token(Lexer *lexer)
{
    skip_whitespace_and_comments(lexer); // skipping
    const char *start = &lexer->source[lexer->index];
    int line = lexer->line;
    if (lexer->curr_char == '\0') {
        return (Token) {TOKEN_EOF, start, 0, line};
    }

    // isalpha(int c) -> Returns a non-zero value (true) if the character is a letter (A-Z or a-z).Returns
    if (isalpha(lexer->curr_char) || lexer->curr_char == '_') {
        return read_identifier_or_keyword(lexer);
    }

    if (isdigit(lexer->curr_char)) {
        return read_number(lexer);
    }

    // Extract Multi-character and Single-character Operators
    char c = lexer->
    switch (c) {
    case '(': return get_single_char_token(lexer, start, line, TOKEN_LPAREN);
    case ')': return get_single_char_token(lexer, start, line, TOKEN_RPAREN);
    case '{': return get_single_char_token(lexer, start, line, TOKEN_LBRACE);
    case '}': return get_single_char_token(lexer, start, line, TOKEN_RBRACE);
    case '[': return get_single_char_token(lexer, start, line, TOKEN_LBRACKET);
    case ']': return get_single_char_token(lexer, start, line, TOKEN_RBRACKET);
    }
    
    //    if (c == '+' || c == '-' || c == '*' || c == '/' || c == '=' || c == ';') {
    //        advance(lexer);
    //        if (c == '=' && lexer->curr_char == '=') {
    //            advance(lexer);
    //        }
    //        int length = (int)(&lexer->source[lexer->index] - start);
    //        return (Token) {TOKEN_OPERATOR, start, length, line};
    //    }

    // Error Token Generation
    advance(lexer);
    return (Token){TOKEN_ERROR, start, 1, line};
}

void display_token(Token token)
{
    const char* type_names[] = {
        "KEYWORD",
        "IDENTIFIER",
        "NUMBER",
        "OPERATOR",
        "EOF",
        "ERROR",
        "LPAREN",
        "RPAREN",
        "LBRACE",
        "RBRACE",
        "LBRACKET",
        "RBRACKET"
    };
    printf("[Line %d] Type: %-10s | Value: \"%.*s\"\n", 
           token.line, type_names[token.type], token.length, token.start);
}

FILE *open_file(const char *path)
{
    return fopen(path, "r");
}

int main()
{
    // now building reading the file
    char *path = "./test.c";
    FILE *file = open_file(path);
    if (file == NULL) {
        perror("Error");
        return 1;
    }

    fseek(file, 0, SEEK_END);
    int file_size = ftell(file);
    rewind(file);

    const char *buff = malloc(file_size * sizeof(char));
    if (buff == NULL) {
        perror("Error");
        return 1;
    }

    size_t bytesRead = fread(buff, 1, file_size, file);

    Lexer lexer;

    init_lexer(&lexer, buff);
    Token token;
    while ((token = get_next_token(&lexer)).type != TOKEN_EOF) {
        display_token(token);
    }
    
    return 0;
}
