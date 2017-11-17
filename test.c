#include "ext2.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main()
{
    int fd;
    int n;
    char buffer[20];

    if((fd = ext2open("/EXT2/ext.h", EXT2_WRONLY | EXT2_CREAT)) == -1) exit(69);
    printf("file descriptor: %d\n", fd);

    ext2seek(fd, 50, EXT2_SEEK_END);
    while((n = ext2read(fd, buffer, 20))) {
        write(1, buffer, n);
    }
    printf("\n");

    ext2close(fd);
    return 0;
}
