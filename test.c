#include "ext2io.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main()
{
    int fd, ext2fd;
    int n;
    char buffer[5000];

    fd = open("ext2.h", O_RDONLY);
    ext2fd = ext2open("/EXT2/ext2.h", EXT2_WRONLY);
    while((n = read(fd, buffer, 5000))) {
        ext2write(ext2fd, buffer, n);
    }

    close(fd);
    ext2close(ext2fd);

    return 0;
}
