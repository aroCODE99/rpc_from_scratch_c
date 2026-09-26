#ifndef GEN_UTILITY_H
#define GEN_UTILITY_H

#include "lexer.h"
#include "ast.h"
#include "emitter.h"

#define SOCKFD "sockfd"
#define NEWFD "new_fd"
#define EXIT_69 69

typedef enum {
    CSIZE_T,
    CINT32_T,
    CUINT32_T,
    CSTRING,
    CVOID,
    CUNKWN,
} CType;

typedef struct {
    CType type;
    const char *name;
    const char *value;
} Statment;

// _Generic is just the switch case
#define TypeName(value) _Generic((value),                 \
        size_t: "size_t", \
        int: "int", \
        float: "float", \
        double: "double", \
        char: "char", \
        char *: "char *", \
        default: "unknown" \
    )

#define generate_stmt(value, name, type)                      \
    generate_stmt_impl(&(value), name, TypeName(value))

typedef enum {
    RPC_INT32,
    RPC_BOOL,
    RPC_STRING,
    RPC_VOID, // don't know what to do with this type
    RPC_UNKNOWN
} RpcType;

int generate_includes(Emitter *);
Statment *generate_stmt_impl(void *value, const char *name, const char *type);
CType classify_ctype(const char *);
RpcType classify_type(Token);
int generate_type(Emitter *, Token);
int generate_method_name(Emitter *, Service *service, Method *method);
int generate_parameter(Emitter *, Parameter *);

#endif
