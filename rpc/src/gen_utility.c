#include "../include/gen_utility.h"

int generate_includes(Emitter *emitter)
{
    emitter_writeln(emitter, "#include <stdio.h>");
    emitter_writeln(emitter, "#include <stdint.h>");
    emitter_writeln(emitter, "#include <stdbool.h>");
    emitter_writeln(emitter, "#include <string.h>");
    emitter_writeln(emitter, "#include <stdlib.h>");
    emitter_writeln(emitter, "#include <unistd.h>");
    emitter_writeln(emitter, "#include <sys/types.h>");
    emitter_writeln(emitter, "#include <sys/socket.h>");
    emitter_writeln(emitter, "#include <netdb.h>");
    emitter_writeln(emitter, "#include <arpa/inet.h>");

    emitter_writeln(emitter, "");
    return 1;
}
CType classify_ctype(const char *type)
{
    if (strcmp(type, "int32_t")) {
        return CINT32_T;
    } else if (strcmp(type, "uint32_t")) {
        return CUINT32_T;
    } else if (strcmp(type, "size_t")) {
        return CSIZE_T;
    }  else if (strcmp(type, "void")) {
        return CVOID;
    } else if (strcmp(type, "char *")) {
        return CSTRING;
    }
    return CUNKWN;
}

int generate_method_name(Emitter *emitter, Service *service, Method* method)
{
    emitter_write_token(emitter, service->name);
    emitter_write(emitter, "_");
    emitter_write_token(emitter, method->name);
    return 1;
}

int generate_type(Emitter *emitter, Token type)
{
    switch (classify_type(type)) {
    case RPC_INT32:
        emitter_write(emitter, "int32_t");
        return 1;
    case RPC_BOOL:
        emitter_write(emitter, "bool");
        return 1;
    case RPC_STRING:
        emitter_write(emitter, "char *");
        return 1;
    case RPC_VOID:
        emitter_write(emitter, "void");
        return 1;
    case RPC_UNKNOWN:
    default:
        log_error("Unknown type '%.*s'", type.length, type.start);
        return 0;
    }
}

RpcType classify_type(Token type)
{
    if (is_keyword(type.start, type.length, "int32")) {
        return RPC_INT32;
    }
    if (is_keyword(type.start, type.length, "bool")) {
        return RPC_BOOL;
    }
    if (is_keyword(type.start, type.length, "string")) {
        return RPC_STRING;
    }
    if (is_keyword(type.start, type.length, "void")) {
        return RPC_VOID;
    }
    return RPC_UNKNOWN;
}


int generate_parameter(Emitter *emitter, Parameter *parameter)
{
    if (!generate_type(emitter, parameter->type)) {
        log_error("Invalid Type");
        return 0;
    }
    emitter_write(emitter, " ");
    emitter_write_token(emitter, parameter->name);
    return 1;
}
