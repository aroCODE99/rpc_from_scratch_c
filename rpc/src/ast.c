#include "../include/ast.h"
#include <string.h>

// validation phase
// now how do i check for the duplicate methods
// a simple 
int validate_service(Service *service)
{
    for (size_t i = 0; i < service->methods.total - 1; ++i) {
        Method *curr_method = service->methods.items[i];
        for (size_t j = i + 1; j < service->methods.total; ++j) {
            Method *other_method = service->methods.items[j];
            if (curr_method->name.length == other_method->name.length &&
                strncmp(curr_method->name.start, other_method->name.start,
                        curr_method->name.length) == 0
                ) {
                    log_error("Semantic error: duplicate method '%.*s'",
                              curr_method->name.length,
                              curr_method->name.start);
                    return 0;
                }
        }
    }
    return 1;
}

static void print_token_node(const char* label, Token token,
        const char* prefix, bool is_last)
{
    // Prints:  ├── Label: token_text   OR   └── Label: token_text
    printf("%s%s %s: ", prefix, is_last ? "└──" : "├──", label);
    display_token(token);
    printf("\n");
}

// i still don't know how does this printing works i just copied it from internet
void print_service_tree(Service *service)
{
    // 1. Root level: Service name
    printf("Service: ");
    display_token(service->name);
    printf("\n");

    size_t total_methods = service->methods.total;
    for (size_t i = 0; i < total_methods; ++i) {
        Method *curr_method = service->methods.items[i];
        bool is_last_method = (i == total_methods - 1);

        // 2. Method level
        print_token_node("Method", curr_method->name, "", is_last_method);

        // Define the prefix for the children of this method.
        // If it's the last method, we don't draw a continuous vertical line down.
        const char* method_prefix = is_last_method ? "    " : "│   ";

        // 3. Inner structure of the method (Parameters & Return Type)
        size_t total_params = curr_method->parameters.total;
        
        // Loop through parameters
        for (size_t j = 0; j < total_params; ++j) {
            Parameter *curr_parameter = curr_method->parameters.items[j];
            
            // Parameter Name
            print_token_node("Param Name", curr_parameter->name, method_prefix, false);
            
            // Parameter Type (nested under the parameter)
            char param_prefix[64];
            snprintf(param_prefix, sizeof(param_prefix), "%s│   ", method_prefix);
            print_token_node("Param Type", curr_parameter->type, param_prefix, true);
        }

        // 4. Return Type (Always at the end of the method's list)
        print_token_node("Return Type", curr_method->return_type, method_prefix, true);
    }
}

// i think this has bug
// this will makes sense to you if u understand how the vector implementation is working
void free_method(Method *method)
{
    for (size_t i = 0; i < method->parameters.total; ++i) {
        // freeeing each parameter
        free(method->parameters.items[i]);
    }
    free(method->parameters.items); // freeeing parameter vector pointer
    free(method);
}

void free_service(Service *service)
{
    for (size_t i = 0; i < service->methods.total; ++i) {
        free_method(service->methods.items[i]);
    }
    free(service->methods.items); // freeing method vector pointer
    free(service);
}
