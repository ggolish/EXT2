#include "ext2.h"

#include <stdio.h>
#include <unistd.h>

int main()
{
    int fd;
    int n;
    char buffer[20];

    fd = ext2open("/mysubdir/fibo.py", EXT2_READ);
    printf("file descriptor: %d\n", fd);

    while((n = ext2read(fd, buffer, 20))) {
        write(1, buffer, 20);
    }
    printf("\n");

    ext2close(fd);
    fd = ext2open("/mysubdir/first.py", EXT2_READ);
    printf("file descriptor: %d\n", fd);

    while((n = ext2read(fd, buffer, 20))) {
        write(1, buffer, 20);
    }
    printf("\n");

    ext2close(fd);
    return 0;
}
