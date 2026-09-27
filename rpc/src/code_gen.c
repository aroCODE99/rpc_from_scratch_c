#include "../include/code_gen.h"

// this is the AST we going through this and emitting the code into the sb
//Service
//    name = UserService
//
//    methods:
//        Method
//            name = GetUser
//            parameters:
//                int32 id
//                string name
//            return_type = User
//
//        Method
//            name = CreateUser
//            parameters:
//                string name
//            return_type = User

int generate_method(Emitter *emitter, Service *service, Method* method)
{
    emitter_writeln(emitter, "// generated method by simple rpc");

    if (!generate_type(emitter, method->return_type)) {
        log_error("Invalid type");
        return 0;
    }
    emitter_write(emitter, " ");

    generate_method_name(emitter, service, method);
    emitter_write(emitter, "(");

    // now generate the params
    // 3
    int n = method->parameters.total;
    for (int i = 0; i < n; ++i) {
        Parameter *params = method->parameters.items[i];
        if (!generate_parameter(emitter, params)) {
            log_error("Error while generating the params");
            return 0;
        }
        if (n - (i + 1) > 0) {
            emitter_write(emitter, ", ");
        }
    }
    emitter_writeln(emitter, ")");
    emitter_open_block(emitter, "");
    emitter_writeln(emitter, "// User Implementation");
    emitter_close_block(emitter, "");
    return 1;
}

static void generate_server_constants(Emitter *emitter)
{
    emitter_writeln(emitter, "#define SERVER_PORT \"9090\"");
    emitter_writeln(emitter, "");
}


static void generate_get_bind_socket_fn(Emitter *emitter)
{
    emitter_writeln(emitter, "static int get_socket(void)");
    emitter_open_block(emitter, "");

    emitter_writeln(emitter, "int %s;", SOCKFD);
    emitter_writeln(
        emitter,
        "struct addrinfo hints, *server_info, *p;"
    );
    emitter_writeln(emitter, "");

    emitter_writeln(emitter, "memset(&hints, 0, sizeof(hints));");
    emitter_writeln(emitter, "hints.ai_family = AF_UNSPEC;");
    emitter_writeln(emitter, "hints.ai_flags = AI_PASSIVE;");
    emitter_writeln(emitter, "hints.ai_socktype = SOCK_STREAM;");
    emitter_writeln(emitter, "");

    emitter_writeln(
        emitter,
        "int return_value = getaddrinfo("
        "NULL, SERVER_PORT, &hints, &server_info);"
    );

    emitter_writeln(emitter, "");

    emitter_open_block(
        emitter,
        "if (return_value != 0)"
    );

    // now this is really confusing
    // %s   → substitute a string
    // %d   → substitute an integer
    // %zu  → substitute a size_t
    // %%   → literally output % -> using this
    emitter_writeln(
        emitter,
        "fprintf(stderr, \"getaddrinfo:%%s\\n\", gai_strerror(return_value));"
    );

    emitter_writeln(emitter, "exit(1);");

    emitter_close_block(emitter, "");

    emitter_writeln(emitter, "");

    emitter_open_block(
        emitter,
        "for (p = server_info; p != NULL; p = p->ai_next)"
    );

    emitter_open_block(
        emitter,
        "if ((%s = socket("
        "p->ai_family, "
        "p->ai_socktype, "
        "p->ai_protocol)) == -1)", SOCKFD
    );

    emitter_writeln(emitter, "perror(\"server: socket\");");
    emitter_writeln(emitter, "continue;");

    emitter_close_block(emitter, "");

    emitter_writeln(emitter, "");

    emitter_writeln(emitter, "int yes = 1;");
    emitter_writeln(emitter, "");

    emitter_open_block(
         emitter, "if (setsockopt(" "%s, " "SOL_SOCKET, " "SO_REUSEADDR, " "&yes, " "sizeof(int)) == -1)",
         SOCKFD
    );

    emitter_writeln(emitter, "perror(\"setsockopt\");");
    emitter_writeln(emitter, "close(%s);", SOCKFD);
    emitter_writeln(emitter, "continue;");

    emitter_close_block(emitter, "");

    emitter_writeln(emitter, "");

    emitter_open_block(
        emitter,
        "if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1)"
    );

    emitter_writeln(emitter, "close(%s);", SOCKFD);
    emitter_writeln(emitter, "perror(\"server: bind\");");
    emitter_writeln(emitter, "continue;");

    emitter_close_block(emitter, "");

    emitter_writeln(emitter, "");

    emitter_writeln(emitter, "break;");

    emitter_close_block(emitter, "");

    emitter_writeln(emitter, "");

    emitter_open_block(
        emitter,
        "if (p == NULL)"
    );

    emitter_writeln(
        emitter,
        "fprintf(stderr, \"server: failed to bind\\n\");"
    );

    emitter_writeln(emitter, "exit(2);");

    emitter_close_block(emitter, "");

    emitter_writeln(emitter, "");

    emitter_writeln(emitter, "freeaddrinfo(server_info);");
    emitter_writeln(emitter, "");

    emitter_writeln(emitter, "return %s;", SOCKFD);

    emitter_close_block(emitter, "");
}

static int generate_method_call(
    Emitter *emitter,
    Service *service,
    Method *method
)
{
    if (!generate_type(emitter, method->return_type)) {
        log_error("Invalid return type");
        return 0;
    }

    emitter_write(emitter, " result = ");

    generate_method_name(emitter, service, method);

    emitter_write(emitter, "(");

    for (size_t i = 0; i < method->parameters.total; ++i) {
        Parameter *parameter = method->parameters.items[i];

        emitter_write_token(emitter, parameter->name);

        if (i + 1 < method->parameters.total) {
            emitter_write(emitter, ", ");
        }
    }

    emitter_writeln(emitter, ");");

    return 1;
}

static int generate_send_response(Emitter *emitter, Method *method)
{
    switch (classify_type(method->return_type)) {
    case RPC_STRING:
        // if it is the string then we first going to send the length as the
        // tcp doesn't have the string mechanics like '\0'
        
        emitter_writeln(emitter, "uint32_t result_len = (uint32_t)strlen(result);");
        emitter_writeln(emitter, "uint32_t net_result_len = htonl(result_len);");
        emitter_writeln(emitter, "");

        emitter_open_block(
            emitter,
            "if (send(%s, &net_result_len, sizeof(net_result_len), 0) <= 0)", NEWFD
        );
        emitter_writeln(emitter, "close(%s);", NEWFD);
        emitter_writeln(emitter, "continue;");
        emitter_close_block(emitter, "");

        emitter_writeln(emitter, "");

        emitter_open_block(
            emitter,
            "if (send(%s, result, result_len, 0) <= 0)", NEWFD
        );
        emitter_writeln(emitter, "close(%s);", NEWFD);
        emitter_writeln(emitter, "continue;");
        emitter_close_block(emitter, "");

        return 1;

    case RPC_INT32:
    case RPC_BOOL:
        /* both fit in a uint32_t on the wire */
        emitter_writeln(emitter, "uint32_t net_result = htonl((uint32_t)result);");
        emitter_writeln(emitter, "");

        emitter_open_block(
            emitter,
            "if (send(%s, &net_result, sizeof(net_result), 0) <= 0)", NEWFD
        );
        emitter_writeln(emitter, "close(%s);", NEWFD);
        emitter_writeln(emitter, "continue;");
        emitter_close_block(emitter, "");
        return 1;
    case RPC_UNKNOWN:
    default:
        log_error("Cannot generate response serialization for unknown type");
        return 0;
    }
}

// so we are currently only supporting the int32_t
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

static int generate_dispatch(Emitter *emitter, Service *service)
{
    const char *method_id = "method_id";
    emitter_writeln(emitter, "uint32_t %s;", method_id);
    emitter_writeln(emitter, "");

    emitter_open_block(
        emitter,
        "if (recv(%s, &%s, sizeof(%s), 0) <= 0)", method_id, method_id, NEWFD
    );

    emitter_writeln(emitter, "close(%s);", NEWFD);
    emitter_writeln(emitter, "continue;");

    emitter_close_block(emitter, "");

    emitter_writeln(emitter, "");

    emitter_writeln(emitter, "%s = unpack(%s);", method_id, method_id);
    emitter_writeln(emitter, "");

    emitter_open_block(emitter, "switch (%s)", method_id);

    // this is we generating the method dispatcher
    for (size_t i = 0; i < service->methods.total; ++i) {
        Method *method = service->methods.items[i];
        emitter_open_block(
            emitter,
            "case %zu:", i
        );

        emitter_writeln(
            emitter,
            "// RPC method"
        );

        // TODO: I think we are repeating this alot
        // generating the parameter for that calling method
        for (size_t j = 0; j < method->parameters.total; ++j) {
            Parameter *parameter = method->parameters.items[j];

            emitter_writeln(emitter, "uint32_t raw_%.*s;",
                            parameter->name.length, parameter->name.start);

            emitter_open_block(emitter, "if (recv(%s, &raw_%.*s, sizeof(uint32_t), 0) <= 0)", NEWFD,
                               parameter->name.length, parameter->name.start);
            emitter_writeln(emitter, "close(%s);", NEWFD);
            emitter_writeln(emitter, "continue;");
            emitter_close_block(emitter, "");

            /* unpacking the raw arg */
            if (!generate_type(emitter, parameter->type)) {
                log_error("Invalid return type");
                return 0;
            }
            emitter_write(emitter, " ");
            emitter_write_token(emitter, parameter->name);
            emitter_write(emitter, " = unpack(raw_%.*s);",
                          parameter->name.length, parameter->name.start);
            emitter_writeln(emitter, "");
        }

        /* calling the method */
        generate_method_call(emitter, service, method);

        if (!generate_send_response(emitter, method)) {
            return 0;
        }
        emitter_writeln(emitter, "break;");
        emitter_close_block(emitter, "");
    }

    emitter_open_block(emitter, "default:");

    emitter_writeln(
        emitter,
        "fprintf(stderr, \"unknown method id: %%u\\n\", %s);", method_id
    );

    emitter_writeln(emitter, "break;");

    emitter_close_block(emitter, "");

    emitter_close_block(emitter, "");

    return 1;
}

static int generate_server_main(Emitter *emitter, Service *service)
{
    emitter_writeln(emitter, "int main(void)");
    emitter_open_block(emitter, "");

    emitter_writeln(
        emitter,
        "int %s = get_socket();", SOCKFD
    );

    emitter_writeln(emitter, "");

    emitter_open_block(emitter, "if (listen(%s, 5) == -1)", SOCKFD);

    emitter_writeln(emitter, "perror(\"listen\");");
    emitter_writeln(emitter, "exit(1);");

    emitter_close_block(emitter,  "");

    emitter_writeln(emitter, "");

    emitter_writeln(
        emitter,
        "printf(\"server waiting for connections\\n\");"
    );

    emitter_writeln(emitter, "");

    emitter_writeln(emitter, "struct sockaddr their_addr;");
    emitter_writeln(emitter, "socklen_t sin_size;");
    emitter_writeln(emitter, "int %s;", NEWFD);
    emitter_writeln(emitter, "");

    emitter_open_block(emitter, "for (;;)");

    emitter_writeln(emitter, "sin_size = sizeof(their_addr);");

    emitter_writeln(
        emitter,
        "%s = accept(%s, "
        "(struct sockaddr *)&their_addr, &sin_size);", NEWFD, SOCKFD
    );

    emitter_writeln(emitter, "");

    emitter_open_block(emitter, "if (%s == -1)", NEWFD);

    emitter_writeln(emitter, "perror(\"accept\");");
    emitter_writeln(emitter, "continue;");

    emitter_close_block(emitter, "");

    emitter_writeln(emitter, "");

    emitter_writeln(emitter, "/* RPC request handling */");
    if (!generate_dispatch(emitter, service)) {
        log_error("Failed to generate dispatch code");
        return 0;
    }

    emitter_writeln(emitter, "");

    emitter_writeln(emitter, "close(%s);", NEWFD);

    emitter_close_block(emitter, "");
    emitter_close_block(emitter, "");

    return 1;
}

int generate_service(Emitter *emitter, Service* service)
{
    if (!generate_includes(emitter)) {
        log_error("Error generating client includes");
        return 0;
    }
    generate_server_constants(emitter);
    generate_unpack_fn(emitter);
    // generating the methods now 
    for (size_t i = 0; i < service->methods.total; ++i) {
        Method *curr_method = service->methods.items[i];
        if (!generate_method(emitter, service, curr_method)) {
            return 0;
        }
        emitter_writeln(emitter, "");
    }

    // networking
    //
    generate_get_bind_socket_fn(emitter);
    if (!generate_server_main(emitter, service)) {
        return 0;
    }
    return 1;
}
