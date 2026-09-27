#include "../include/utility.h"

static void print_help(const char *program_name)
{
    printf(
        "Usage: %s [options] <input.rpc>\n"
        "\n"
        "Generate C client and/or server code from an RPC definition file.\n"
        "\n"
        "Options:\n"
        "  -o, --output <dir>    Output directory (default: .)\n"
        "      --client         Generate client code only\n"
        "      --server         Generate server code only\n"
        "      --ast            Print the parsed AST and exit\n"
        "  -h, --help            Show this help message\n"
        "\n"
        "Notes:\n"
        "  If neither --client nor --server is specified, both are generated.\n"
        "  --client and --server cannot be used together.\n"
        "  --ast prints the parsed AST instead of generating code.\n"
        "\n"
        "Examples:\n"
        "  %s service.rpc\n"
        "  %s --client service.rpc\n"
        "  %s --server service.rpc\n"
        "  %s -o generated service.rpc\n"
        "  %s --ast service.rpc\n"
        "\n",
        program_name,
        program_name,
        program_name,
        program_name,
        program_name,
        program_name
    );
}

char *read_whole_file_in_buffer(const char *path)
{
    if (path == NULL) {
        log_error("Path cannot be NULL");
        return NULL;
    }
    FILE *file = fopen(path, "r");
    if (file == NULL) {
        log_error("Failed to open file: %s", path);
        return NULL;
    }

    if (fseek(file, 0, SEEK_END) < 0) {
        log_error("Failed fseek");
        return NULL;
    }
    int file_size = ftell(file);
    rewind(file); // i am not sure if i need this

    char *buff = malloc(file_size * sizeof(char));
    if (buff == NULL) {
        log_error("Failed to allocate size to the buffer");
        return NULL;
    }

    size_t bytes_read = fread(buff, 1, file_size, file);
    if (bytes_read > 0) return buff;
    buff[bytes_read] = '\0';
    return NULL;
}

static char* shift_args(int *argc, char ***argv)
{
    if ((*argc) < 1) {
        return NULL;
    }
    *argc -= 1;
    return *(*argv)++;
}

void normalize_output_dir(char *output_dir)
{
    size_t len = strlen(output_dir);

    while (len > 1 && output_dir[len - 1] == '/') {
        output_dir[len - 1] = '\0';
        len--;
    }
}

int parse_args(int argc, char **argv, Options *options)
{
    options->program_name = shift_args(&argc, &argv); // skipping the programm name
    while (argc > 0) {
        char *curr_arg = shift_args(&argc, &argv);
        if (curr_arg[0] == '-') {
            if (strcmp(curr_arg, "--ast") == 0) {
                options->print_ast = true;
            } else if (strcmp(curr_arg, "--client") == 0) {
                options->client = true;
                options->server = false;
            } else if (strcmp(curr_arg, "--server") == 0) {
                options->client = false;
                options->server = true;
            } else if (strcmp(curr_arg, "--help") == 0 || strcmp(curr_arg, "-h") == 0) {
                print_help(options->program_name);
                return 0;
            } else if (strcmp(curr_arg, "--output") == 0 || strcmp(curr_arg, "-o") == 0) {
                if (argc < 1) {
                    fprintf(stderr, "option %s: requires parameter\n", curr_arg);
                    return 0;
                }
                options->output_dir = shift_args(&argc, &argv);
                normalize_output_dir(options->output_dir);
            } else {
                fprintf(stderr, "error: unknown option '%s'\n", curr_arg);
                return 0;
            }
        } else {
            if (options->input != NULL) {
                fprintf(stderr,
                    "error: multiple input files specified: '%s'\n",
                     curr_arg
                );
                return 0;
            }
            options->input = curr_arg;
        }
    }
    return 1;
}

Service *compile_file(const char *file_name)
{
    char *buff = read_whole_file_in_buffer(file_name); // reading whole file in the buffer

    if (buff == NULL) {
        return NULL;
    }

    Lexer lexer = {0};
    init_lexer(&lexer, buff);

    Parser parser = {0};
    init_parser(&parser, &lexer); // intializing the parserg
    Service *service = parse_service(&parser);
    
    if (!validate_service(service)) {
        log_error("Invalid Service");
        free_service(service);
        free(buff);
        return NULL;
    }

    return service;
}

// so i could remove this path thing
// and have the (Options *)
// first of all what is the path
// "options->generated_path/name"

int write_output_file(const char *output_dir,
        const char *file_name,
        const char *content
) {
    // but how does this method figure out what we are generating the client or server
    // TODO: validate the file_name;
    if (output_dir == NULL || file_name == NULL || content == NULL) {
        return 0;
    }

    size_t n = strlen(output_dir) + strlen(file_name) + 2; //one for null termination and other for the slash
    char path[n];
    snprintf(path, n, "%s/%s", output_dir, file_name);

    // so there is no checking for the dir here if it exists or not
    FILE *file = fopen(path, "w");
    if (file == NULL) {
        perror("fopen");
        return 0;
    }

    fputs(content, file);

    fclose(file);
    return 1;
}

int generate_files(Service *service, Options *options)
{
    // if we just need to display the ast
    int status = 1;
    Emitter server_emitter = {0};
    Emitter client_emitter = {0};

    if (options->print_ast) {
        print_service_tree(service);
        status = 0;
        goto cleanup;
    }
    
    if (options->server) {
        if (!emitter_init(&server_emitter)) {
            log_error("Failed to initialize emitter");
            goto cleanup;
        }
    
        if (!generate_service(&server_emitter, service)) {
            log_error("Code generation failed");
            goto cleanup;
        }

        if (!write_output_file(options->output_dir, "rpc_server.c", server_emitter.sb.buff)) {
            log_error("Failed to write file");
            goto cleanup;
        }
    }

    if (options->client) {
        if (!emitter_init(&client_emitter)) {
            log_error("Failed to initialize emitter");
            goto cleanup;
        }
    
        if (!generate_client(&client_emitter, service)) {
            log_error("Code generation failed");
            goto cleanup;
        }

        if (!write_output_file(options->output_dir, "rpc_client.c", client_emitter.sb.buff)) {
            log_error("Failed to write file");
            goto cleanup;
        }
    }
    
    status = 0; // everything went as expected

cleanup:
    emitter_free(&client_emitter);
    emitter_free(&server_emitter);

    free_service(service);

    return status;
}
