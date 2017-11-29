#include "ext2.h"
#include "ext2io.h"
#include "utility.h"

int ext2close(int fd)
{
    if(!ext2checkfs()) {
        error_msg("Filesystem not initialized!");
        return -1;
    }
    
    return ext2_delete_file(fd);
}

