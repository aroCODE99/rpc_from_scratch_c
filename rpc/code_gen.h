#ifndef CODE_GEN_H
#define CODE_GEN_H

#include "emitter.h"
#include "ast.h"
#include <stdio.h>

int generate_service(Emitter *emitter, Service *service);

#endif
