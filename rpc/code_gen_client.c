#include "code_gen_client.h"

#define PACK_TYPE "uint32_t"

#define generate_send_response_raw(emitter, value, name) \
    generate_send_response_raw_impl(emitter, &(value), name, TypeName(value))

#define generate_recv_respone_raw(emitter, value, name) \
    generate_recv_respone_raw_impl(emitter, &value, name, TypeName(value))

// just for the sake of it 
static int generate_main(Emitter *emitter)
{
    emitter_writeln(emitter, "int main()");
    emitter_open_block(emitter, "");
    emitter_writeln(emitter, "return 0;");
    emitter_close_block(emitter, "");
    return 1;
}

static int generate_client_constants(Emitter *emitter)
{
    emitter_writeln(
        emitter,
        "#define SERVER_PORT \"9090\""
    );

    emitter_writeln(
        emitter,
        "#define SERVER \"127.0.0.1\""
    );

    emitter_writeln(emitter, "");

    return 1;
}


static int generate_rpc_client_struct(Emitter *emitter)
{
    emitter_writeln(emitter, "typedef struct {");
    emitter_writeln(emitter, "    int sockfd;");
    emitter_writeln(emitter, "} RpcClient;");
    emitter_writeln(emitter, "");
    return 1;
}

static int generate_unpack_fn(Emitter *emitter)
{
    emitter_writeln(
        emitter,
        "static int32_t unpack(uint32_t num)"
    );

    emitter_open_block(emitter, "");

    emitter_writeln(
        emitter,
        "return ntohl((int32_t)num);"
    );

    emitter_close_block(emitter, "");

    emitter_writeln(emitter, "");

    return 1;
}

static int generate_pack_fn(Emitter *emitter)
{
    emitter_writeln(
        emitter,
        "static uint32_t pack(uint32_t value)"
    );

    emitter_open_block(emitter, "");

    emitter_writeln(
        emitter,
        "return htonl(value);"
    );

    emitter_close_block(emitter, "");

    emitter_writeln(emitter, "");

    return 1;
}

static int generate_get_socket_fn(Emitter *emitter)
{
    emitter_writeln(
        emitter,
        "static int get_socket(void)"
    );

    emitter_open_block(emitter, "");

    emitter_writeln(
        emitter,
        "int sockfd;"
    );

    emitter_writeln(
        emitter,
        "struct addrinfo hints, *server_info, *p;"
    );

    emitter_writeln(emitter, "");

    emitter_writeln(
        emitter,
        "memset(&hints, 0, sizeof(hints));"
    );

    emitter_writeln(
        emitter,
        "hints.ai_family = AF_UNSPEC;"
    );

    emitter_writeln(
        emitter,
        "hints.ai_socktype = SOCK_STREAM;"
    );

    emitter_writeln(emitter, "");

    emitter_writeln(
        emitter,
        "int return_value = getaddrinfo("
        "SERVER, "
        "SERVER_PORT, "
        "&hints, "
        "&server_info);"
    );

    emitter_writeln(emitter, "");

    emitter_open_block(
        emitter,
        "if (return_value != 0)"
    );

    emitter_writeln(
        emitter,
        "fprintf(stderr, "
        "\"getaddrinfo: %%s\\n\", "
        "gai_strerror(return_value));"
    );

    emitter_writeln(
        emitter,
        "exit(%zu);", EXIT_69
    );

    emitter_close_block(emitter, "");

    emitter_writeln(emitter, "");

    emitter_open_block(
        emitter,
        "for (p = server_info; "
        "p != NULL; "
        "p = p->ai_next)"
    );

    emitter_open_block(
        emitter,
        "if ((sockfd = socket("
        "p->ai_family, "
        "p->ai_socktype, "
        "p->ai_protocol)) == -1)"
    );

    emitter_writeln(
        emitter,
        "perror(\"client: socket\");"
    );

    emitter_writeln(
        emitter,
        "continue;"
    );

    emitter_close_block(emitter, "");

    emitter_open_block(
        emitter,
        "if (connect("
        "sockfd, "
        "p->ai_addr, "
        "p->ai_addrlen) == -1)"
    );

    emitter_writeln(
        emitter,
        "perror(\"client: connect\");"
    );

    emitter_writeln(
        emitter,
        "close(sockfd);"
    );

    emitter_writeln(
        emitter,
        "continue;"
    );

    emitter_close_block(emitter, "");

    emitter_writeln(
        emitter,
        "break;"
    );

    emitter_close_block(emitter, "");

    emitter_writeln(emitter, "");

    emitter_open_block(
        emitter,
        "if (p == NULL)"
    );

    emitter_writeln(
        emitter,
        "fprintf(stderr, "
        "\"client: failed to connect\\n\");"
    );

    emitter_writeln(
        emitter,
        "freeaddrinfo(server_info);"
    );

    emitter_writeln(
        emitter,
        "exit(%zu);", EXIT_69
    );

    emitter_close_block(emitter, "");

    emitter_writeln(emitter, "");

    emitter_writeln(
        emitter,
        "freeaddrinfo(server_info);"
    );

    emitter_writeln(emitter, "");

    emitter_writeln(
        emitter,
        "return sockfd;"
    );

    emitter_close_block(emitter, "");

    emitter_writeln(emitter, "");

    return 1;
}

static int generate_rpc_client_connect(Emitter *emitter)
{
    emitter_writeln(
        emitter,
        "static int rpc_client_connect(RpcClient *client)"
    );

    emitter_open_block(emitter, "");

    emitter_writeln(
        emitter,
        "client->sockfd = get_socket();"
    );

    emitter_writeln(
        emitter,
        "return 1;"
    );

    emitter_close_block(emitter, "");

    emitter_writeln(emitter, "");

    return 1;
}


static int generate_rpc_client_close(Emitter *emitter)
{
    emitter_writeln(
        emitter,
        "static void rpc_client_close(RpcClient *client)"
    );

    emitter_open_block(emitter, "");

    emitter_writeln(
        emitter,
        "close(client->sockfd);"
    );

    emitter_close_block(emitter, "");

    emitter_writeln(emitter, "");

    return 1;
}


// This method should pack the sending
// i also need the name

// so the methodId is the just the index of the vector based methods
// let's suppose i am sending this
// int32_t method_id = 0;
//  (type)  (name)     (value)

// how do i make this generic
// so the packing is fix which is "uint32_t
// for now let's think in terms of the int and bools
static int generate_send_response_raw_impl(
        Emitter *emitter, void *value, const char *name, const char *type
) {
    emitter_writeln(emitter, "// Sending the ", name);
    size_t val = *((size_t *)value);
    emitter_writeln(emitter, "%s %s = %zu;", type, name, val);
 
    // suffixing it with the pack_name
    emitter_writeln(emitter, "%s pack_%s = pack(%s);",
        PACK_TYPE,
        name,
        name
    );
    
    // and now sending it
    emitter_open_block(
        emitter,
        "if (!send(client->%s, &pack_%s, sizeof(pack_%s), 0) <= 0)", SOCKFD,
        name, name
    );
    emitter_writeln(emitter, "close(client->%s);", SOCKFD);
    emitter_writeln(emitter, "exit(%zu);", EXIT_69);
    emitter_close_block(emitter, "");
    return 1;
}

int generate_parameters(Emitter *emitter, Method *method)
{
    emitter_write(emitter, "(RpcClient *client"); // Fixed argument in the client method's
    
    for (size_t i = 0; i < method->parameters.total; ++i) {
        Parameter *parameter = method->parameters.items[i];
        emitter_write(emitter, ", ");
        if (!generate_parameter(emitter, parameter)) {
            log_error("Error generating the parameters");
            return 0;
        }
    }
    emitter_writeln(emitter, ")");
    return 1;
}

static int generate_send_parameter_response(Emitter *emitter, Parameter *parameter)
{
    emitter_writeln(emitter, "// Something");
    char name[parameter->name.length + 1];
    snprintf(name, sizeof(name), "%.*s", parameter->name.length, parameter->name.start);
    // packing the args
    if (!generate_type(emitter, parameter->type)) {
        log_error("error: generating the return type");
        return 0;
    }

    emitter_writeln(emitter, " pack_%s;", name);
    emitter_open_block(emitter,
         "if (!send(client->%s, &pack_%s, sizeof(pack_%s), 0))",
         SOCKFD, name, name
     );
    emitter_writeln(emitter, "close(client->%s);", SOCKFD);
    emitter_writeln(emitter, "exit(%zu);", EXIT_69);
     emitter_close_block(emitter, "");
    return 1;
}

static int generate_send_parameters_response(Emitter *emitter, Method *method)
{
    for (size_t i = 0; i < method->parameters.total; ++i) {
        Parameter *curr_param = method->parameters.items[i];
        if (!generate_send_parameter_response(emitter, curr_param)) {
            return 0;
        }
    }
    return 1;
}

static int generate_method_return_expr(Emitter *emitter, Method *method)
{
    emitter_writeln(emitter,
        "%s raw_res;", PACK_TYPE,
        method->name.length, method->name.start);
    emitter_open_block(emitter, "if (recv(client->%s, &raw_res, sizeof(raw_res), 0) <= 0)", SOCKFD);
    emitter_writeln(emitter, "close(client->%s);", SOCKFD);
    emitter_writeln(emitter, "exit(%zu);", EXIT_69);
    emitter_close_block(emitter, "");

    emitter_write(emitter, "return (");
    if (!generate_type(emitter, method->return_type)) {
        log_error("Error: Generating the return type");
        return 0;
    }
    emitter_write(emitter, ")");
    emitter_writeln(emitter, "unpack(raw_res);");
    
    return 1;
}

static int generate_rpc_method(Emitter *emitter, Service *service, size_t method_id)
{
    Method *method = service->methods.items[method_id];

    if (method == NULL) {
        log_error("Invalid methodId");
        return 0;
    }

    if (!generate_type(emitter, method->return_type)) {
        log_error("Error generating the return_type");
        return 0;
    }

    emitter_write(emitter, " ");
    
    if (!generate_method_name(emitter, service, method)) {
        log_error("Error generating the method_name");
        return 0;
    }
    if (!generate_parameters(emitter, method)) {
        log_error("Error: generating the parameters");
        return 0;
    }

    emitter_open_block(emitter, "");

    emitter_writeln(emitter, "// RPC client call");

    // now pack the method id and send it
    // i need this to be a very generic method
    if (!generate_send_response_raw(emitter, method_id, "methodId")) {
        log_error("Error generating the send method_id");
        return 0;
    }

    if (!generate_send_parameters_response(emitter, method)) {
        log_error("Error generating the sending response");
        return 0;
    }

    if (!generate_method_return_expr(emitter, method)) {
        log_error("Error generating the Return response");
        return 0;
    }
    emitter_close_block(emitter, "");

    emitter_writeln(emitter, "");

    return 1;
}


int generate_rpc_methods(Emitter *emitter, Service *service)
{
    for (size_t i = 0; i < service->methods.total; ++i) {
        if (!generate_rpc_method(emitter, service, i)) {
            log_error("Error while generating the methods");
            return 0;
        }
    }
    return 1;
}


int generate_client(Emitter *emitter, Service *service)
{
    if (!generate_includes(emitter)) {
        log_error("Error generating client includes");
        return 0;
    }

    if (!generate_client_constants(emitter)) {
        return 0;
    }

    if (!generate_rpc_client_struct(emitter)) {
        return 0;
    }

    if (!generate_pack_fn(emitter)) {
        return 0;
    }
    
    if (!generate_unpack_fn(emitter)) {
        return 0;
    }

    if (!generate_get_socket_fn(emitter)) {
        return 0;
    }

    if (!generate_rpc_client_connect(emitter)) {
        return 0;
    }

    if (!generate_rpc_client_close(emitter)) {
        return 0;
    }

    if (!generate_rpc_methods(emitter, service)) {
        return 0;
    }

    if (!generate_main(emitter)) {
        return 0;
    }

    return 1;
}
