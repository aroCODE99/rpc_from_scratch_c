#include "lexer.h"
#include <stdio.h>
#include <stdlib.h>

typedef struct {
    char *name;
    char *type;
} Parameter;

typedef struct {
    char *name;
    Parameter *parameters;
    int parameter_count;
    char *return_type;
} Method;

typedef struct {
    char *name;
    Method *methods;
    int method_count;
} Service;
 
int main()
{
    // now building reading the file
    char *path = "./test.c";
    FILE *file = fopen(path, "r");
    if (file == NULL) {
        perror("Error");
        return 1;
    }

    fseek(file, 0, SEEK_END);
    int file_size = ftell(file);
    rewind(file);

    char *buff = malloc(file_size * sizeof(char));
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
