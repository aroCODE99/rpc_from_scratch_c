#ifndef LEXER_H
#define LEXER_H

#include <stdio.h>
#include <stddef.h>

/*
 * All possible token types produced by the lexer.
 */
typedef enum {
    TOKEN_KEYWORD,
    TOKEN_IDENTIFIER,
    TOKEN_NUMBER,
    TOKEN_EOF,
    TOKEN_ERROR,

    // Operators
    TOKEN_PLUS,
    TOKEN_MINUS,

    // Parentheses
    TOKEN_LPAREN,
    TOKEN_RPAREN,

    // Braces
    TOKEN_LBRACE,
    TOKEN_RBRACE,

    // Brackets
    TOKEN_LBRACKET,
    TOKEN_RBRACKET,

    // Separators
    TOKEN_COMMA,
    TOKEN_SEMICOLON

} TokenType;


/*
 * Represents a single token.
 */
typedef struct {
    TokenType type;

    /*
     * Pointer to the beginning of the token
     * inside the original source string.
     */
    const char *start;

    /*
     * Number of characters in the token.
     */
    int length;

    /*
     * Line where the token appeared.
     */
    int line;

} Token;


/*
 * Represents the state of the lexer.
 */
typedef struct {
    const char *source;
    char curr_char;
    int index;
    int line;

} Lexer;


/*
 * Initialize the lexer with source code.
 */
void init_lexer(Lexer *lexer, const char *source);


/*
 * Move to the next character in the source.
 */
void advance(Lexer *lexer);


/*
 * Look at the character after the current character
 * without consuming it.
 */
char peek(Lexer *lexer);


/*
 * Skip whitespace and // comments.
 */
void skip_whitespace_and_comments(Lexer *lexer);


/*
 * Get the next token from the source.
 */
Token get_next_token(Lexer *lexer);


/*
 * Display a token for debugging.
 */
void display_token(Token token);

#endif
