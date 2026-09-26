#ifndef CODE_GEN_CLIENT_H
#define CODE_GEN_CLIENT_H

#include "emitter.h"
#include "ast.h"
#include "gen_utility.h"
#include <stdio.h>

int generate_client(Emitter *emitter, Service *service);

#endif
