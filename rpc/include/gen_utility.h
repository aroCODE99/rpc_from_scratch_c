#ifndef GEN_UTILITY_H
#define GEN_UTILITY_H

#include "lexer.h"
#include "ast.h"
#include "emitter.h"
#include <stdint.h>
#include <stdbool.h>

#define SOCKFD "sockfd"
#define CLIENTFD "fd"
#define EXIT_69 69

typedef enum {
    CSIZE_T,
    CINT32_T,
    CUINT32_T,
    CSTRING,
    CVOID,
    CBOOL,
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
        int32_t: "int32_t", \
        uint32_t: "uint32_t", \
        bool: "bool", \
        char: "char", \
        char *: "char *", \
        default: "unknown" \
    )

#define generate_stmt(value, name, type)                      \
    generate_stmt_impl(&(value), name, TypeName(value))

typedef enum {
    INT32,
    UINT32,
    BOOL,
    STRING,
    UNKNOWN
} RpcType;

void generate_get_fn(Emitter *emitter, const char *, RpcType);
void generate_get_fn_token(Emitter *emitter, Token, Token);
int generate_includes(Emitter *);
Statment *generate_stmt_impl(void *value, const char *name, const char *type);
CType classify_ctype(const char *);
RpcType classify_type(Token);
const char *get_c_type(RpcType);
const char *get_rpc_type_string(RpcType);
int generate_type(Emitter *, Token);
int generate_method_name(Emitter *, Service *service, Method *method);
int generate_parameter(Emitter *, Parameter *);
int generate_rpc_serialize_method(Emitter *, const char *, const char *);
int generate_rpc_serialize_method_token(Emitter *, Token, Token);
int generate_rpc_deserialize_method_token(Emitter *, Token);
int generate_send_all_fn(Emitter *emitter);
int generate_recv_all_fn(Emitter *emitter);
const char *get_default_value(Method *method);
#endif
