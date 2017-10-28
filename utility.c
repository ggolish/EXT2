#include "utility.h"

#include <stdio.h>

void *safe_malloc(size_t size)
{
    void *dest;

    if((dest = malloc(size)) == NULL) {
        warning("Malloc failed!");
    }

    return dest;
}

void warning(const char *errmesg)
{
    fprintf(stderr, "\033[31mWarning:\0330m %s\n", errmesg);
}
