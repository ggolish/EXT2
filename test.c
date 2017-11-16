#include "ext2.h"

#include <stdio.h>

int main()
{
    int fd;

    fd = ext2open("/mysubdir/fibo.py", EXT2_READ);
    printf("file descriptor: %d\n", fd);
    ext2close(fd);
    fd = ext2open("/mysubdir/first.py", EXT2_READ);
    printf("file descriptor: %d\n", fd);
    return 0;
}
