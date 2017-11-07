#include "utility.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

void *safe_malloc(size_t size, const char *errmsg)
{
    void *dest;

    if((dest = malloc(size)) == NULL) {
        die(errmsg);
    }

    return dest;
}

void *safe_realloc(void *ptr, size_t size, const char *errmsg)
{
    void *dest;

    if((dest = realloc(ptr, size)) == NULL) {
        die(errmsg); 
    }

    return dest;
}

int safe_open(const char *filename, int flags, const char *errmsg)
{
    int fd;

    if((fd = open(filename, flags)) == -1) {
        die(errmsg);
    }

    return fd;
}

ssize_t safe_read(int fd, void *buf, size_t count, const char *errmsg)
{
    ssize_t rv;

    if((rv = read(fd, buf, count)) == -1) {
        die(errmsg);
    }

    return rv;
}

char *safe_getenv(const char *name)
{
    char *res;
    char *errmsg;

    if((res = getenv(name)) == NULL) {
        errmsg = (char *)safe_malloc((strlen(name) + 50) * sizeof(char), "");
        sprintf(errmsg, "Unable to load environment variable '%s'!", name);
        die(errmsg);
    }

    return res;
}

void error_msg(const char *err)
{
    fprintf(stderr, "\033[31m%s\033[0m\n", err);
}

void die(const char *errmsg)
{
    fprintf(stderr, "\033[31mFatal:\033[0m %s\n", errmsg);
    exit(69);
}
