#ifndef UTILITY_H_
#define UTILITY_H_

#include <stdlib.h>

extern void *safe_malloc(size_t size, const char *errmsg);
extern int safe_open(const char *pathname, int flags, const char *errmsg);
extern ssize_t safe_read(int fd, void *buf, size_t count, const char *errmsg);
extern void die(const char *errmsg);

#endif
