#include "parser.h"
#include "ast.h"
#include "emitter.h"
#include "code_gen.h"

#include <stdio.h>

int write_output_file(const char *path, const char *content)
{
    FILE *file = fopen(path, "w");

    if (file == NULL) {
        perror("fopen");
        return 0;
    }

    fputs(content, file);

    fclose(file);
    return 1;
}

int main()
{
    char *buff = read_whole_file_in_buffer(); // reading whole file in the buffer
    Lexer lexer = {0};
    init_lexer(&lexer, buff);
    logger_set_level(LOG_INFO);
 
    Parser parser = {0};
    init_parser(&parser, &lexer); // intializing the parser
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
        log_error("Cde generation failed");
        emitter_free(&emitter);
        free_service(service);
        free(buff);
        return 1;
    }


    if (!write_output_file("generated.c", emitter.sb.buff)) {
        log_error("Failed to write file");
        emitter_free(&emitter);
        free_service(service);
        free(buff);
        return 1;
    }

    free_service(service);
    free(buff);
    return 0;
}
