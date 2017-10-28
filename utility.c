#include "utility.h"

#include <fcntl.h>
#include <stdio.h>
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

void die(const char *errmsg)
{
    fprintf(stderr, "\033[31mFatal:\0330m %s\n", errmsg);
    exit(69);
}
