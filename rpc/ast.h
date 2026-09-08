#ifndef AST_H
#define AST_H

#include "lexer.h"
#include "vector.h"

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

void display_service(Service *service);
void free_service(Service *service);

#endif
