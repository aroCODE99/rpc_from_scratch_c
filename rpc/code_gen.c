#include "code_gen.h"

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


static int generate_type(Emitter *emitter, Token type)
{
    if (is_keyword(type.start, type.length, "int32")) {
        emitter_write(emitter, "int32_t");
        return 1;
    }
    if (is_keyword(type.start, type.length, "bool")) {
        emitter_write(emitter, "bool");
        return 1;
    }
    if (is_keyword(type.start, type.length, "string")) {
        emitter_write(emitter, "char *");
        return 1;
    }
    log_error(
        "Unknown type '%.*s'",
        type.length,
        type.start
    );
    return 0;
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

int generate_method(Emitter *emitter, Service *service, Method* method)
{
    emitter_writeln(emitter, "// generated method by simple rpc");

    if (!generate_type(emitter, method->return_type)) {
        log_error("Invalid type");
        return 0;
    }
    emitter_write(emitter, " ");

    emitter_write_token(emitter, service->name);
    emitter_write(emitter, "_");
    emitter_write_token(emitter, method->name);
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
    emitter_write(emitter, ")");
    emitter_open_block(emitter, "");
    emitter_writeln(emitter, "// User Implementation");
    emitter_close_block(emitter, "");
    return 1;
}

int generate_service(Emitter *emitter, Service* service)
{
    emitter_writeln(emitter, "#include <stdint.h>");
    emitter_writeln(emitter, "#include <stdbool.h>");
    emitter_writeln(emitter, "");

    // generating the methods now 
    for (int i = 0; i < service->methods.total; ++i) {
        Method *curr_method = service->methods.items[i];
        if (!generate_method(emitter, service, curr_method)) {
            return 0;
        }
        emitter_writeln(emitter, "");
    }
    return 1;
}
