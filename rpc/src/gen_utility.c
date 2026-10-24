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
    emitter_writeln(emitter, "#define RPC_BUFFER_IMPLEMENTATION");
    emitter_writeln(emitter, "#include \"serialization.h\"");
    emitter_writeln(emitter, "#define RPC_HELPER_IMPLEMENTATION");
    emitter_writeln(emitter, "#include \"rpc_helper.h\"");

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
    } else if (strcmp(type, "bool")) {
        return CBOOL;
    } else return CUNKWN;
}

int generate_method_name(Emitter *emitter, Service *service, Method* method)
{
    emitter_write_token(emitter, service->name);
    emitter_write(emitter, "_");
    emitter_write_token(emitter, method->name);
    return 1;
}

// very specific use case could replace it with the get_c_type(RpcType)
int generate_type(Emitter *emitter, Token type)
{
    switch (classify_type(type)) {
    case INT32:
        emitter_write(emitter, "int32_t ");
        return 1;
    case BOOL:
        emitter_write(emitter, "bool ");
        return 1;
    case STRING:
        emitter_write(emitter, "char *");
        return 1;
    case UNKNOWN:
    default:
        log_error("Unknown type '%.*s'", type.length, type.start);
        return 0;
    }
}

int generate_variable_declaration(Emitter *emitter, Token type, Token name)
{
    if (!generate_type(emitter, type)) {
        return 0;
    }
    emitter_write_token(emitter, name);
    emitter_writeln(emitter, ";");
    return 1;
}

RpcType classify_type(Token type)
{
    if (is_keyword(type.start, type.length, "int32")) {
        return INT32;
    }
    if (is_keyword(type.start, type.length, "bool")) {
        return BOOL;
    }
    if (is_keyword(type.start, type.length, "string")) {
        return STRING;
    }
    return UNKNOWN;
}

const char *get_c_type(RpcType type)
{
    switch (type) {
    case INT32:
        return "int32_t";
    case UINT32:
        return "uint32_t";
    case BOOL:
        return "bool";
    case STRING:
        return "char *";
    default:
        return NULL;
    }
}

const char *get_rpc_type_string(RpcType type)
{
    switch (type) {
    case INT32:
        return "INT32";
    case UINT32:
        return "UINT32";
    case BOOL:
        return "BOOL";
    case STRING:
        return "STRING";
    default:
        return NULL;
    }
}

int generate_parameter(Emitter *emitter, Parameter *parameter)
{
    if (!generate_type(emitter, parameter->type)) {
        log_error("Invalid Type");
        return 0;
    }
    emitter_write_token(emitter, parameter->name);
    return 1;
}

int generate_rpc_serialize_method(Emitter *emitter, const char *type, const char *name)
{
    if (strcmp(type, "int32_t") == 0) {
        emitter_writeln(emitter, "rpc_serialize_i32(&buffer, %s);", name);
    } else if (strcmp(type, "uint32_t") == 0) {
        emitter_writeln(emitter, "rpc_serialize_u32(&buffer, %s);", name);
    } else if (strcmp(type, "size_t") == 0) {
        emitter_writeln(emitter, "rpc_serialize_u32(&buffer, %s);", name);
    } else if (strcmp(type, "char *") == 0) {
        emitter_writeln(emitter, "rpc_serialize_string(&buffer, %s);", name);
    } else if (strcmp(type, "bool") == 0) {
        emitter_writeln(emitter, "rpc_serialize_bool(&buffer, %s);", name);
    } else return 0;
    return 1;
}

static bool token_match(Token tok, const char *type)
{
    int n = strlen(type);
    if (tok.length != n) {
        return false;
    }
    return strncmp(tok.start, type, tok.length) == 0;
}

/*
  you still need to close the block and the add the termination expr
 */
int generate_rpc_deserialize_method(Emitter *emitter, Token type, const char *name)
{
    if (token_match(type, "int32")) {
        emitter_open_block(emitter, "if (rpc_deserialize_i32(&buffer, &%s) < 0) ", name);
    } else if (token_match(type, "uint32")) {
        emitter_open_block(emitter, "if (rpc_deserialize_u32(&buffer, &%s) < 0) ", name);
    } else if (token_match(type, "string")) {
        emitter_open_block(emitter, "if (rpc_deserialize_string(&buffer, &%s) < 0) ", name);
    } else if (token_match(type, "bool")) {
        emitter_open_block(emitter, "if (rpc_deserialize_bool(&buffer, &%s) < 0) ", name);
    } else {
        return 0;
    }

    emitter_writeln(
        emitter,
        "fprintf(stderr, \"Error: failed to deserialize parameter %s\\n\");",
        name
    );

    return 1;
}

int generate_rpc_deserialize_method_token(Emitter *emitter, Token type, Token name)
{
    char name_str[name.length + 1];
    memcpy(name_str, name.start, name.length);
    name_str[name.length] = '\0';

    return generate_rpc_deserialize_method(emitter, type, name_str);
}

// matches the rpc_types
int generate_rpc_serialize_method_token(Emitter *emitter, Token type, Token name)
{
    if (token_match(type, "int32")) {
        emitter_write(emitter, "rpc_serialize_i32(&buffer, ");
        emitter_write_token(emitter, name);
        emitter_writeln(emitter, ");");
    } else if (token_match(type, "uint32")) {
        emitter_write(emitter, "rpc_serialize_u32(&buffer, ");
        emitter_write_token(emitter, name);
        emitter_writeln(emitter, ");");
    } else if (token_match(type, "string")) {
        emitter_write(emitter, "rpc_serialize_string(&buffer, ");
        emitter_write_token(emitter, name);
        emitter_writeln(emitter, ");");
    } else if (token_match(type, "bool") == 0) {
        emitter_write(emitter, "rpc_serialize_bool(&buffer, ", name);
        emitter_write_token(emitter, name);
        emitter_writeln(emitter, ");");
    } else return 0;
    return 1;
}

int generate_send_all_fn(Emitter *emitter)
{
    emitter_writeln(
        emitter,
        "static int send_all(RpcClient *client, RpcBuffer *buffer)"
    );

    emitter_open_block(emitter, "");

    emitter_writeln(emitter, "size_t sent = 0;");
    emitter_writeln(emitter, "");

    emitter_open_block(
        emitter,
        "while (sent < buffer->length)"
    );

    emitter_writeln(
        emitter,
        "size_t n = send(client->sockfd,buffer->data + sent, buffer->length - sent, 0);"
    );

    emitter_writeln(emitter, "");

    emitter_open_block(emitter, "if (n < 0)");

    emitter_open_block(emitter, "if (errno == EINTR)");
    emitter_writeln(emitter, "continue;");
    emitter_close_block(emitter, "");

    emitter_writeln(
        emitter,
        "fprintf(stderr, \"Failed to send data\\n\");"
    );

    emitter_writeln(emitter, "return 0;");

    emitter_close_block(emitter, "");

    emitter_open_block(emitter, "if (n == 0)");

    emitter_writeln(emitter, "return 0;");

    emitter_close_block(emitter, "");

    emitter_writeln(
        emitter,
        "sent += (size_t)n;"
    );

    emitter_close_block(emitter, "");

    emitter_writeln(emitter, "return 1;");

    emitter_close_block(emitter, "");
    emitter_writeln(emitter, "");

    return 1;
}

int generate_recv_all_fn(Emitter *emitter)
{
    emitter_writeln(emitter,
        "static int recv_all(int sockfd, void *buffer, size_t length)");

    emitter_open_block(emitter, "");

    emitter_writeln(emitter,
        "unsigned char *data = buffer;");
    emitter_writeln(emitter, "size_t total = 0;");
    emitter_writeln(emitter, "");

    emitter_open_block(emitter, "while (total < length)");

    emitter_writeln(emitter,
        "ssize_t received = recv("
        "sockfd, data + total, length - total, 0);");

    emitter_open_block(emitter, "if (received < 0)");
    emitter_open_block(emitter, "if (errno == EINTR)");
    emitter_writeln(emitter, "continue;");
    emitter_close_block(emitter, "");
    emitter_writeln(emitter, "return -1;");
    emitter_close_block(emitter, "");

    emitter_open_block(emitter, "if (received == 0)");
    emitter_writeln(emitter, "return -1;");
    emitter_close_block(emitter, "");

    emitter_writeln(emitter,
        "total += (size_t)received;");

    emitter_close_block(emitter, "");

    emitter_writeln(emitter, "return 0;");

    emitter_close_block(emitter, "");
    emitter_writeln(emitter, "");

    return 1;
}

const char *get_default_value(Method *method)
{
    if (method == NULL) {
        return "0";
    }

    if (token_match(method->return_type, "bool")) {
        return "false";
    }

    if (token_match(method->return_type, "string")) {
        return "NULL";
    }

    // int32, int64, and other numeric types
    return "0";
}

void generate_get_fn(Emitter *emitter, const char *name, RpcType type)
{
    const char *fn;

    switch (type) {
    case INT32:  fn = "get_i32";    break;
    case UINT32: fn = "get_u32";   break;
    case BOOL:   fn = "get_bool";  break;
    case STRING: fn = "get_string"; break;
    default:
        log_error("Error: Unknown type");
        return;
    }

    emitter_open_block(
        emitter,
        "if (%s(fd, &%s) < 0) ",
        fn,
        name
    );


    emitter_writeln(
        emitter,
        "fprintf(stderr, \"Error: Failed to deserialize field '%.*s'.\\n\");",
        strlen(name), name
    );
    emitter_writeln(emitter, "return 0;");
    emitter_close_block(emitter, "");
}

void generate_get_fn_token(Emitter *emitter, Token name, Token type)
{
    char name_buf[name.length + 1];
    memcpy(name_buf, name.start, name.length);
    name_buf[name.length] = '\0';
    generate_get_fn(emitter, name_buf, classify_type(type));
}


static void generate_serialized_size(Emitter *emitter, Parameter *parameter, bool returns)
{
    switch (classify_type(parameter->type)) {
    case INT32:
    case UINT32:
    case BOOL:
        emitter_write(emitter, "sizeof(uint32_t)");
        break;
    case STRING:
        if (returns) emitter_write(emitter, "sizeof(uint32_t) + strlen(result)");
        else {
            emitter_write(
                emitter,
                "sizeof(uint32_t) + strlen(%.*s)",
                parameter->name.length,
                parameter->name.start
            );
        }
        break;
    default:
        log_error("Error: Unknown type");
    }
}

// TODO: replace the 'Method *method' with the Token name, Token type
int generate_total_len(Emitter *emitter, Method *method, bool client, bool returns)
{
    // [4-byte payload length][method_id][Args]
    // [string length][string bytes]...
    emitter_write(emitter, "uint32_t total_len = ");

    if (client) {
        emitter_write(emitter, "sizeof(method_id) + ");
    }

    for (size_t i = 0; i < method->parameters.total; ++i) {
        Parameter *curr_param = method->parameters.items[i];
        if (i > 0) emitter_write(emitter, " + ");
        generate_serialized_size(emitter, curr_param, returns);
    }

    emitter_writeln(emitter, ";");
    return 1;
}
