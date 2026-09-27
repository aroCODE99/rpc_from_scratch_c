#ifndef CODE_GEN_H
#define CODE_GEN_H

#include "emitter.h"
#include "ast.h"
#include "gen_utility.h"
#include <stdio.h>

// i don't know if i should do this
typedef struct {
    Emitter emitter;
} GenContext;

int generate_service(Emitter *emitter, Service *service);

#endif
