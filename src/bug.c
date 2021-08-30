#include "bug.h"
#include <stdio.h>
#include <stdlib.h>

void __bug(char const *file, int line, char const *cond)
{
    fprintf(stderr, "BUG: %s:%d: %s\n", file, line, cond);
    fflush(stderr);
    exit(EXIT_FAILURE);
}
