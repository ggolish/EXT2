#include "ext2.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main()
{
    int fd;
    int n;
    char buffer[20];

    if((fd = ext2open("/EXT2/ext2.h", EXT2_READ)) == -1) exit(69);
    printf("file descriptor: %d\n", fd);

    while((n = ext2read(fd, buffer, 20))) {
        write(1, buffer, n);
    }
    printf("\n");

    ext2close(fd);
    return 0;
}
