#include "../include/parser.h"
#include "../include/ast.h"
#include "../include/emitter.h"
#include "../include/code_gen.h"
#include "../include/code_gen_client.h"
#include "../include/utility.h"

#include <stdio.h>

int main(int argc, char **argv)
{
    // setting up the default options
    Options options = {
        .client = true,
        .server = true,
        .output_dir = "."
    };
    // ./rpc_gen -o test_rpc
    // parsing the arg

    // TODO: pointing out the error in the file
    if (!parse_args(argc, argv, &options)) {
        exit(2);
    }

    if (options.input == NULL) {
        fprintf(stderr, "error: no input file specified\n");
        fprintf(stderr, "try '%s --help' for more information\n", options.program_name);
        exit(2);
    }
    
    Service *service = compile_file(options.input);

    int status = generate_files(service, &options);
    return status;
}
