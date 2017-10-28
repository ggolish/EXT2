#ifndef EXT2IO_H_
#define EXT2IO_H_

#define EXT2_READ   0x1
#define EXT2_WRITE  0x2

#include "ext2.h"

typedef struct {
    int fd;
    int flags;
} EXT2_FILE;

extern EXT2_FILE *ext2open(EXT2 *fs, const char *pathname, int flags);
extern void ext2close(EXT2_FILE *ext2fd);

#endif
