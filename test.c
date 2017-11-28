#include "ext2.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main()
{
    int fd;
    int n;
    char buffer[500];

    if((fd = ext2open("/EXT2/ext2.h", EXT2_WRONLY | EXT2_CREAT)) == -1) exit(69);
    printf("file descriptor: %d\n", fd);

    n = ext2read(fd, buffer, 500);
    write(1, buffer, n);
    printf("\n");

    ext2seek(fd, 0, EXT2_SEEK_SET);

    printf("return value: %d\n", ext2write(fd, "Does this work?", 15));

    ext2seek(fd, 0, EXT2_SEEK_SET);

    n = ext2read(fd, buffer, 500);
    write(1, buffer, n);
    printf("\n");

//     ext2write(fd, "Testing", 450);

    ext2close(fd);
    return 0;
}
