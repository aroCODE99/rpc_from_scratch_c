#include "../include/generate_serialization_header.h"

static void generate_ensure_capacity(Emitter *emitter)
{
    emitter_writeln(
        emitter,
        "static int rpc_buffer_ensure_capacity(RpcBuffer *buf, size_t add_len)"
    );

    emitter_open_block(emitter, "");

    emitter_writeln(
        emitter,
        "if (!buf || add_len > SIZE_MAX - buf->length - 1) return 0;"
    );
    emitter_writeln(
        emitter,
        "size_t req_cap = buf->length + add_len + 1;"
    );
    emitter_writeln(emitter, "");

    emitter_open_block(emitter, "if (req_cap <= buf->capacity) ");
    emitter_writeln(emitter, "return 1; // which is allowed");
    emitter_close_block(emitter, "");
    emitter_writeln(emitter, "");

    emitter_writeln(
        emitter,
        "// initialize this shit"
    );
    emitter_writeln(emitter, "");
    emitter_writeln(
        emitter,
        "/*"
    );
    emitter_writeln(
        emitter,
        "  void *realloc(void *_Nullable p, size_t size);"
    );
    emitter_writeln(
        emitter,
        "  If p is NULL, then the call is equivalent to malloc(size),"
    );
    emitter_writeln(
        emitter,
        "  for all values of size."
    );
    emitter_writeln(
        emitter,
        "*/"
    );

    emitter_writeln(emitter, "size_t new_cap = buf->capacity ? buf->capacity: BUF_INIT_CAPACITY;");

    emitter_open_block(
        emitter,
        "while (new_cap < req_cap) "
    );
    emitter_writeln(emitter, "new_cap *= 2;");
    emitter_close_block(emitter, "");
    emitter_writeln(emitter, "");

    // generating the imp comment
    emitter_writeln(
        emitter,
        "/*"
    );
    emitter_writeln(
        emitter,
        "  If we used req_cap directly, realloc would happen"
    );
    emitter_writeln(
        emitter,
        "  more frequently."
    );
    emitter_writeln(
        emitter,
        "*/"
    );

    emitter_writeln(
        emitter,
        "uint8_t *data = realloc(buf->data, new_cap);"
    );

    emitter_open_block(
        emitter,
        "if (data == NULL) "
    );

    emitter_writeln(
        emitter,
        "fprintf(stderr, \"Failed to realloc\\n\");"
    );
    emitter_writeln(
        emitter,
        "return 0;"
    );

    emitter_close_block(emitter, "");
    emitter_writeln(emitter, "");

    emitter_writeln(
        emitter,
        "buf->data = data;"
    );
    emitter_writeln(
        emitter,
        "buf->capacity = new_cap;"
    );
    emitter_writeln(emitter, "");

    emitter_writeln(
        emitter,
        "return 1;"
    );
    
    emitter_close_block(emitter, "");
}

static void generate_rpc_buffer_free(Emitter *emitter)
{
    emitter_writeln(emitter, "void rpc_buffer_free(RpcBuffer *buf)");
    emitter_open_block(emitter, "");
    emitter_open_block(emitter, "if (buf->data) ");
    emitter_writeln(emitter, "free(buf->data);");
    emitter_writeln(emitter, "buf->data = NULL;");
    emitter_close_block(emitter, "");
    
    emitter_writeln(emitter, "buf->length = 0;");
    emitter_writeln(emitter, "buf->capacity = 0;");
    emitter_writeln(emitter, "buf->position = 0;");

    emitter_close_block(emitter, "");
}

static void generate_rpc_append_primitive(Emitter *emitter)
{
    emitter_writeln(emitter, "static int rpc_buffer_append_primitive(RpcBuffer *buff, void *data, size_t len)");
    emitter_open_block(emitter, "");
    
    emitter_open_block(emitter, "if (!rpc_buffer_ensure_capacity(buff, len)) ");
    emitter_writeln(emitter, "return 0;");
    emitter_close_block(emitter, "");

    emitter_writeln(emitter, "memcpy(buff->data + buff->length, data, len);");
    emitter_writeln(emitter, "buff->length += len;");
    emitter_writeln(emitter, "return 1;");
    
    emitter_close_block(emitter, "");
}

static void generate_rpc_read_primitive(Emitter *emitter)
{
    emitter_writeln(
        emitter,
        "static int rpc_buffer_read_primitive(RpcBuffer *buf, void *out, size_t len)"
    );

    emitter_open_block(emitter, "");

    emitter_writeln(emitter, "if (!buf || !out) return 0;");
    emitter_writeln(emitter, "// Bounds checking");
    
    emitter_open_block(
        emitter,
        "if (buf->position > buf->length || len > buf->length - buf->position) "
    );
    emitter_writeln(emitter, "return 0;");
    emitter_close_block(emitter, "");

    emitter_writeln(
        emitter,
        "memcpy(out, buf->data + buf->position, len);"
    );
    emitter_writeln(emitter, "buf->position += len;");
    emitter_writeln(emitter, "return 1;");

    emitter_close_block(emitter, "");
}

static int generate_rpc_serialize_u32(Emitter *emitter)
{
    emitter_writeln(emitter, "int rpc_serialize_u32(RpcBuffer *buf, uint32_t val)");
    emitter_open_block(emitter, "");
    emitter_writeln(emitter, "uint32_t packed_val = htonl(val);");

    emitter_open_block(emitter, "if (!rpc_buffer_append_primitive(buf, &packed_val, sizeof(packed_val)))");
    emitter_writeln(emitter, "return 0;");
    emitter_close_block(emitter, "");

    emitter_writeln(emitter, "return 1;");
    emitter_close_block(emitter, "");
    return 1;
}

static int generate_rpc_deserialize_u32(Emitter *emitter)
{
    emitter_writeln(
        emitter,
        "int rpc_deserialize_u32(uint32_t packed_val, uint32_t *out)"
    );
    emitter_open_block(emitter, "");

    emitter_open_block(emitter, "if (!out) ");
    emitter_writeln(emitter, "fprintf(stderr, \"Error: Invalid serialized uint32_t value\\n\");");
    emitter_writeln(emitter, "return 0;");
    emitter_close_block(emitter, "");

    
    emitter_writeln(emitter, "*out = ntohl(packed_val);");
    emitter_writeln(emitter, "return 1;");
    emitter_close_block(emitter, "");
    emitter_writeln(emitter, "");
    return 1;
}

static int generate_rpc_serialize_i32(Emitter *emitter)
{
    emitter_writeln(emitter, "int rpc_serialize_i32(RpcBuffer *buf, int32_t val)");
    emitter_open_block(emitter, "");
    emitter_writeln(emitter, "uint32_t packed_val;");
    emitter_writeln(emitter, "memcpy(&packed_val, &val, sizeof(packed_val)); "
                  "// copying the exact bits of int32_t to uint32_t");
    emitter_writeln(emitter, "packed_val = htonl(packed_val);");

    emitter_open_block(emitter, "if (!rpc_buffer_append_primitive(buf, &packed_val, sizeof(packed_val)))");
    emitter_writeln(emitter, "return 0;");
    emitter_close_block(emitter, "");

    emitter_writeln(emitter, "return 1;");
    emitter_close_block(emitter, "");
    return 1;
}

static int generate_rpc_deserialize_i32(Emitter *emitter)
{
    emitter_writeln(
        emitter,
        "int rpc_deserialize_i32(uint32_t packed_val, int32_t *out)"
    );

    emitter_open_block(emitter, "");

    emitter_open_block(emitter, "if (!out) ");
    emitter_writeln(emitter, "fprintf(stderr, \"Error: Invalid serialized int32_t value\\n\");");
    emitter_writeln(emitter, "return 0;");
    emitter_close_block(emitter, "");

    
    emitter_writeln(emitter, "packed_val = ntohl(packed_val);");
    emitter_writeln(emitter, "memcpy(out, &packed_val, sizeof(*out));");
    emitter_writeln(emitter, "return 1;");

    emitter_close_block(emitter, "");
    emitter_writeln(emitter, "");
    return 1;
}

static int generate_rpc_serialize_bool(Emitter *emitter)
{
    emitter_writeln(emitter, "int rpc_serialize_bool(RpcBuffer *buff, bool val)");
    emitter_open_block(emitter, "");
    emitter_writeln(emitter, "uint32_t packed_val = val == true ? htonl(1) : htonl(0);");
    
    emitter_open_block(emitter, "if (!rpc_buffer_append_primitive(buff, &packed_val, sizeof(packed_val))) ");
    emitter_writeln(emitter, "return 0;");
    emitter_close_block(emitter, "");
    
    emitter_writeln(emitter, "return 1;");
    emitter_close_block(emitter, "");
    return 1;
}

static int generate_rpc_deserialize_bool(Emitter *emitter)
{
    emitter_writeln(
        emitter,
        "int rpc_deserialize_bool(uint32_t packed_val, bool *out)"
    );

    emitter_open_block(emitter, "");

    emitter_open_block(emitter, "if (!out) ");
    emitter_writeln(emitter, "fprintf(stderr, \"Error: Invalid serialized boolean value\\n\");");
    emitter_writeln(emitter, "return 0;");
    emitter_close_block(emitter, "");

    emitter_writeln(emitter, "uint32_t val = ntohl(packed_val);");

    emitter_open_block(emitter, "if (val > 1) ");
    emitter_writeln(emitter, "fprintf(stderr, \"Error: Invalid serialized boolean value.\\n\");");
    emitter_writeln(emitter, "return 0;");
    emitter_close_block(emitter, "");

    emitter_writeln(emitter, "*out = (val == 1);");
    emitter_writeln(emitter, "return 1;");

    emitter_close_block(emitter, "");
    return 1;
}

static int generate_rpc_serialize_string(Emitter *emitter)
{
    emitter_writeln(emitter, "int rpc_serialize_string(RpcBuffer *buf, const char *str)");
    emitter_open_block(emitter, "");

    emitter_writeln(emitter, "if (!buf || !str) return 0;");
    emitter_writeln(emitter, "size_t str_len = strlen(str);");
    emitter_open_block(emitter, "if (str_len > UINT32_MAX)");
    emitter_writeln(emitter, "return 0;");
    emitter_close_block(emitter, "");
    emitter_writeln(emitter, "uint32_t len = (uint32_t)str_len;");
    emitter_writeln(emitter, "uint32_t packed_len = htonl(len); // Convert to network byte order");
    emitter_writeln(emitter, "");
    emitter_writeln(emitter, "// Ensure capacity for: sizeof(len) + len");
    emitter_writeln(emitter, "size_t total_needed = sizeof(uint32_t) + len;");
    emitter_open_block(emitter, "if (!rpc_buffer_ensure_capacity(buf, total_needed))");
    emitter_writeln(emitter, "return 0;");
    emitter_close_block(emitter, "");
    emitter_writeln(emitter, "");
    emitter_writeln(emitter, "// Copy the length prefix");
    emitter_writeln(emitter, "memcpy(buf->data + buf->length, &packed_len, sizeof(uint32_t));");
    emitter_writeln(emitter, "buf->length += sizeof(uint32_t);");
    emitter_writeln(emitter, "");
    emitter_writeln(emitter, "// Copy the string characters");
    emitter_writeln(emitter, "memcpy(buf->data + buf->length, str, len);");
    emitter_writeln(emitter, "");
    emitter_writeln(emitter, "buf->length += len;");
    emitter_writeln(emitter, "return 1;");
    
    emitter_close_block(emitter, "");
    return 1;
}

static int generate_rpc_deserialize_string(Emitter *emitter)
{
    emitter_writeln(
        emitter,
        "int rpc_deserialize_string(RpcBuffer *buf, char **out)"
    );

    emitter_open_block(emitter, "");

    emitter_writeln(emitter, "if (!buf || !out) return 0;");
    emitter_writeln(emitter, "*out = NULL;");
    emitter_writeln(emitter, "uint32_t packed_len;");

    emitter_open_block(
        emitter,
        "if (!rpc_buffer_read_primitive("
        "buf, &packed_len, sizeof(packed_len)))"
    );
    emitter_writeln(emitter, "return 0;");
    emitter_close_block(emitter, "");

    emitter_writeln(emitter, "uint32_t len = ntohl(packed_len);");

    emitter_open_block(
        emitter,
        "if (buf->position > buf->length || "
        "(size_t)len > buf->length - buf->position)"
    );
    emitter_writeln(emitter, "return 0;");
    emitter_close_block(emitter, "");

    emitter_open_block(
        emitter,
        "if (len == UINT32_MAX)"
    );
    emitter_writeln(emitter, "return 0;");
    emitter_close_block(emitter, "");

    emitter_writeln(
        emitter,
        "char *str = malloc((size_t)len + 1);"
    );

    emitter_open_block(emitter, "if (!str)");
    emitter_writeln(emitter, "return 0;");
    emitter_close_block(emitter, "");

    emitter_open_block(
        emitter,
        "if (!rpc_buffer_read_primitive(buf, str, len))"
    );
    emitter_writeln(emitter, "free(str);");
    emitter_writeln(emitter, "return 0;");
    emitter_close_block(emitter, "");

    emitter_writeln(emitter, "str[len] = '\\0';");
    emitter_writeln(emitter, "*out = str;");
    emitter_writeln(emitter, "return 1;");

    emitter_close_block(emitter, "");
    return 1;
}

int generate_sb_header_file(Emitter *emitter)
{
    emitter_writeln(emitter, "#ifndef RPC_BUFFER_H");
    emitter_writeln(emitter, "#define RPC_BUFFER_H");
    emitter_writeln(emitter, "");

    emitter_writeln(emitter, "#include <stddef.h>");
    emitter_writeln(emitter, "#include <stdint.h>");
    emitter_writeln(emitter, "#include <stdbool.h>");
    emitter_writeln(emitter, "#include <stdio.h>");
    emitter_writeln(emitter, "#include <stdlib.h>");
    emitter_writeln(emitter, "#include <string.h>");
    emitter_writeln(emitter, "#include <arpa/inet.h>");

    emitter_writeln(emitter, "");

    emitter_open_block(emitter, "typedef struct");
    emitter_writeln(emitter, "uint8_t *data;");
    emitter_writeln(emitter, "size_t length;");
    emitter_writeln(emitter, "size_t capacity;");
    emitter_writeln(emitter, "size_t position;");
    emitter_close_block(emitter, " RpcBuffer;");
    emitter_writeln(emitter, "");

    emitter_writeln(emitter, "#define BUF_INIT_CAPACITY 64");
    emitter_writeln(emitter, "");

    emitter_writeln(emitter, "void rpc_buffer_free(RpcBuffer *);");
    emitter_writeln(emitter, "int rpc_serialize_u32(RpcBuffer *, uint32_t);");
    emitter_writeln(emitter, "int rpc_serialize_i32(RpcBuffer *, int32_t);");
    emitter_writeln(emitter, "int rpc_serialize_bool(RpcBuffer *, bool);");
    emitter_writeln(emitter, "int rpc_serialize_string(RpcBuffer *, const char *);");

    // deserialization methods
    emitter_writeln(emitter, "int rpc_deserialize_u32(uint32_t packed_val, uint32_t *out);");
    emitter_writeln(emitter, "int rpc_deserialize_i32(uint32_t packed_val, int32_t *out);");
    emitter_writeln(emitter, "int rpc_deserialize_bool(uint32_t packed_val, bool *out);");
    emitter_writeln(emitter, "int rpc_deserialize_string(RpcBuffer *buf, char **out);");
    

    emitter_writeln(emitter, "");

    emitter_writeln(emitter, "#ifdef RPC_BUFFER_IMPLEMENTATION");
    emitter_writeln(emitter, "");

    generate_rpc_buffer_free(emitter);
    emitter_writeln(emitter, "");
    generate_ensure_capacity(emitter);
    emitter_writeln(emitter, "");
    generate_rpc_append_primitive(emitter);
    emitter_writeln(emitter, "");
    generate_rpc_read_primitive(emitter);
    emitter_writeln(emitter, "");
    generate_rpc_serialize_u32(emitter);
    emitter_writeln(emitter, "");
    generate_rpc_serialize_i32(emitter);
    emitter_writeln(emitter, "");
    generate_rpc_serialize_bool(emitter);
    emitter_writeln(emitter, "");
    generate_rpc_serialize_string(emitter);
    emitter_writeln(emitter, "");
    generate_rpc_deserialize_u32(emitter);
    emitter_writeln(emitter, "");
    generate_rpc_deserialize_i32(emitter);
    emitter_writeln(emitter, "");
    generate_rpc_deserialize_bool(emitter);
    emitter_writeln(emitter, "");
    generate_rpc_deserialize_string(emitter);

    emitter_writeln(emitter, "");
   
    emitter_writeln(emitter, "#endif // IMPLEMENTATION");
    emitter_writeln(emitter, "#endif // HEADER");

    return 1;
}
