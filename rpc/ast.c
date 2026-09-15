#include "ast.h"
#include <string.h>

// validation phase
// now how do i check for the duplicate methods
// a simple 
int validate_service(Service *service)
{
    for (int i = 0; i < service->methods.total - 1; ++i) {
        Method *curr_method = service->methods.items[i];
        for (int j = i + 1; j < service->methods.total; ++j) {
            Method *other_method = service->methods.items[j];
            if (curr_method->name.length == other_method->name.length &&
                strncmp(curr_method->name.start, other_method->name.start,
                        curr_method->name.length) == 0)
                {
                    log_error("Semantic error: duplicate method '%.*s'",
                              curr_method->name.length,
                              curr_method->name.start);
                    return 0;
                }
        }
    }
    return 1;
}

void display_service(Service *service)
{
    // just displaying the name
    display_token(service->name);
    // printing methods
    for (int i = 0; i < service->methods.total; ++i) {
        Method *curr_method = service->methods.items[i];
        display_token(curr_method->name);
        // displaying the parameter
        for (int j = 0; j < curr_method->parameters.total; ++j) {
            Parameter *curr_parameter = curr_method->parameters.items[j];
            display_token(curr_parameter->name);
            display_token(curr_parameter->type);
        }
        display_token(curr_method->return_type);
    }
}

// i think this has bug
// this will makes sense to you if u understand how the vector implementation is working
void free_method(Method *method)
{
    for (int i = 0; i < method->parameters.total; ++i) {
        // freeeing each parameter
        free(method->parameters.items[i]);
    }
    free(method->parameters.items); // freeeing parameter vector pointer
    free(method);
}

void free_service(Service *service)
{
    for (int i = 0; i < service->methods.total; ++i) {
        free_method(service->methods.items[i]);
    }
    free(service->methods.items); // freeing method vector pointer
    free(service);
}
