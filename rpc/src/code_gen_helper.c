#include "../include/code_gen_helper.h"

static int generate_recv_rpc_buffer(Emitter *emitter)
{
    emitter_writeln(emitter, "int recv_rpc_buffer(int sockfd, RpcBuffer *buffer)");
    
    emitter_open_block(emitter, "");
    emitter_writeln(emitter, "uint32_t network_length;");
    
    emitter_open_block(emitter, "if (!buffer) ");
    emitter_writeln(emitter, "fprintf(stderr, \"Error: RPC buffer cannot be NULL.\\n\");");
    emitter_writeln(emitter, "return -1;");
    emitter_close_block(emitter, "");

    emitter_open_block(emitter, "if (recv_all(sockfd, &network_length, sizeof(network_length)) < 0) ");
    emitter_writeln(emitter, "return -1;");
    emitter_close_block(emitter, "");

    emitter_writeln(emitter, "uint32_t payload_length = ntohl(network_length);");
    emitter_open_block(emitter, "if (!rpc_buffer_ensure_capacity(buffer, payload_length)) ");
    emitter_writeln(emitter, "return -1;");
    emitter_close_block(emitter, "");
    
    emitter_open_block(emitter, "if (recv_all(sockfd, buffer->data, payload_length) < 0) ");
    emitter_writeln(emitter, "return -1;");
    emitter_close_block(emitter, "");

    emitter_writeln(emitter, "buffer->length = payload_length;");
    emitter_writeln(emitter, "buffer->position = 0;");

    emitter_writeln(emitter, "return 0;");

    emitter_close_block(emitter, "");
    emitter_writeln(emitter, "");
    
    return 1;
}

static int generate_send_all_fn(Emitter *emitter)
{
    emitter_writeln(
        emitter,
        "int send_all(int sockfd, RpcBuffer *buffer)"
    );
    emitter_open_block(emitter, "");

    emitter_open_block(emitter, "if (sockfd < 0 || !buffer || (!buffer->data && buffer->length)) ");
    emitter_writeln(emitter, "fprintf(stderr, \"Error: Invalid arguments provided to send operation.\\n\");");
    emitter_writeln(emitter, "return 0;");
    emitter_close_block(emitter, "");
    emitter_writeln(emitter, "");

    emitter_writeln(emitter, "uint32_t sent = 0;");
    emitter_open_block(emitter, "while (sent < buffer->length) ");
    emitter_writeln(
        emitter,
        "ssize_t n = send(sockfd, buffer->data + sent, buffer->length - sent, 0);"
    );
    emitter_writeln(emitter, "");

    emitter_open_block(emitter, "if (n < 0) ");
    emitter_open_block(emitter, "if (errno == EINTR) ");
    emitter_writeln(emitter, "continue;");
    emitter_close_block(emitter, "");
    emitter_writeln(emitter, "fprintf(stderr, \"ERROR: Network operation failed: %%s (errno: %%d)\\n\", strerror(errno), errno);");
    emitter_writeln(emitter, "return 0;");
    emitter_close_block(emitter, "");

    emitter_open_block(emitter, "if (n == 0) ");
    emitter_writeln(emitter, "return 0;");
    emitter_close_block(emitter, "");

    emitter_writeln(emitter, "sent += (uint32_t)n;");
    emitter_close_block(emitter, "");

    emitter_writeln(emitter, "return sent;");
    emitter_close_block(emitter, "");
    emitter_writeln(emitter, "");

    return 1;
}

static int generate_recv_all_fn(Emitter *emitter)
{
    // recv_all method
    emitter_writeln(
        emitter,
        "int recv_all(int sockfd, void *data, uint32_t length)"
    );
    emitter_open_block(emitter, "");

    emitter_writeln(emitter, "uint32_t received = 0;");
    emitter_writeln(emitter, "");

    emitter_open_block(emitter, "if (!data && length) ");
    emitter_writeln(emitter, "fprintf(stderr, \"Error: Data cannot be null while adding data\\n\");");
    emitter_writeln(emitter, "return 0;");
    emitter_close_block(emitter, "");

    emitter_writeln(emitter, "");

    emitter_open_block(emitter, "while (received < length) ");
    emitter_writeln(
        emitter,
        "ssize_t n = recv(sockfd, data + received, length - received, 0);"
    );
    emitter_writeln(emitter, "");

    emitter_open_block(emitter, "if (n < 0) ");
    emitter_open_block(emitter, "if (errno == EINTR) ");
    emitter_writeln(emitter, "continue;");
    emitter_close_block(emitter, "");
    emitter_writeln(emitter, "fprintf(stderr, \"ERROR: Network operation failed: %%s (errno: %%d)\\n\", strerror(errno), errno);");
    emitter_writeln(emitter, "return 0;");
    emitter_close_block(emitter, "");
    emitter_open_block(emitter, "if (n == 0) ");
    emitter_writeln(emitter, "fprintf(stderr, \"Connection closed while receiving data\\n\");");
    emitter_writeln(emitter, "return 0;");
    emitter_close_block(emitter, "");

    emitter_writeln(emitter, "received += (uint32_t)n;");
    emitter_close_block(emitter, "");


    emitter_writeln(emitter, "return received;");
    emitter_close_block(emitter, "");
    emitter_writeln(emitter, "");
    return 1;
}

int generate_rpc_helper_file(Emitter *emitter)
{
    emitter_writeln(emitter, "#ifndef RPC_HELPER_H");
    emitter_writeln(emitter, "#define RPC_HELPER_H");
    emitter_writeln(emitter, "");

    emitter_writeln(emitter, "#include <errno.h>");
    emitter_writeln(emitter, "#include <stddef.h>");
    emitter_writeln(emitter, "#include <stdio.h>");
    emitter_writeln(emitter, "#include <stdint.h>");
    emitter_writeln(emitter, "#include <sys/types.h>");
    emitter_writeln(emitter, "#include <sys/socket.h>");
    emitter_writeln(emitter, "#include \"serialization.h\"");
    emitter_writeln(emitter, "");

    emitter_open_block(emitter, "typedef enum ");
    emitter_writeln(emitter, "INT32, UINT32, BOOL, STRING");
    emitter_close_block(emitter, " RPC_TYPES;");
    emitter_writeln(emitter, "");
    
    emitter_open_block(emitter, "typedef struct ");
    emitter_writeln(emitter, "int sockfd;");
    emitter_close_block(emitter, " RpcClient;");
    emitter_writeln(emitter, "");

    emitter_writeln(emitter, "int recv_rpc_buffer(int, RpcBuffer *);");
    emitter_writeln(emitter, "int send_all(int, RpcBuffer *);");
    emitter_writeln(emitter, "int recv_all(int, void *, uint32_t);");
    
    emitter_writeln(emitter, "");

    emitter_writeln(emitter, "#ifdef RPC_HELPER_IMPLEMENTATION");
    emitter_writeln(emitter, "");

    // recv_rpc_buffer method
    if (!generate_recv_rpc_buffer(emitter)) {
        return 0;
    }

    // send_all method
    if (!generate_send_all_fn(emitter)) {
        return 0;
    }

    // recv_all method
    if (!generate_recv_all_fn(emitter)) {
        return 0;
    }

    emitter_writeln(emitter, "#endif // RPC_HELPER_IMPLEMENTATION");
    emitter_writeln(emitter, "#endif // RPC_HELPER_H");

    return 1;
}

