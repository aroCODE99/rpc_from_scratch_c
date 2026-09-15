#ifndef UTILITY_H
#define UTILITY_H

#include "logger.h"
#include <stdlib.h>

char *read_whole_file_in_buffer(const char *);
int write_output_file(const char *, const char *);
char *shift_args(int *, char ***);

#endif
