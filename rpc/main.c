#include "parser.h"
#include "ast.h"
#include "emitter.h"
#define GEN_UTILITY_IMPLEMENTATION
#include "code_gen.h"
#include "code_gen_client.h"
#include "utility.h"

#include <stdio.h>

int main(int argc, char **argv)
{
    shift_args(&argc, &argv); // shifting the program name
    const char *file_name = shift_args(&argc, &argv);
    printf("%s\n", file_name);
    char *buff = read_whole_file_in_buffer(file_name); // reading whole file in the buffer

    if (buff == NULL) {
        return 1;
    }

    Lexer lexer = {0};
    init_lexer(&lexer, buff);
    logger_set_level(LOG_INFO);
 
    Parser parser = {0};
    init_parser(&parser, &lexer); // intializing the parserg
    Service *service = parse_service(&parser);
    
    if (!validate_service(service)) {
        log_error("Invalid Service");
        free_service(service);
        free(buff);
        return 1;
    }

    Emitter emitter = {0};
    if (!emitter_init(&emitter)) {
        log_error("Failed to initialize emitter");
        free_service(service);
        free(buff);
        return 1;
    }
    
    if (!generate_service(&emitter, service)) {
        log_error("Code generation failed");
        emitter_free(&emitter);
        free_service(service);
        free(buff);
        return 1;
    }

    if (!write_output_file("rpc_server.c", emitter.sb.buff)) {
        log_error("Failed to write file");
        emitter_free(&emitter);
        free_service(service);
        free(buff);
        return 1;
    }
    emitter_free(&emitter);

    Emitter client_emitter = {0};
    if (!emitter_init(&client_emitter)) {
        log_error("Failed to initialize emitter");
        free_service(service);
        free(buff);
        return 1;
    }
    
    if (!generate_client(&client_emitter, service)) {
        log_error("Code generation failed");
        emitter_free(&client_emitter);
        free_service(service);
        free(buff);
        return 1;
    }

    if (!write_output_file("rpc_client.c", client_emitter.sb.buff)) {
        log_error("Failed to write file");
        emitter_free(&emitter);
        free_service(service);
        free(buff);
        return 1;
    }

    emitter_free(&client_emitter);
    free_service(service);
    free(buff);
    return 0;
}
