#include "ast.h"

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

void free_service(Service *service)
{
    
}
    
