#include "ext2.h"
#include "utility.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>

int ext2read(int fd, char *buf, int count)
{
    EXT2_FILE *ext2fd;
    int toread, bytesread, offset;
    int i;
    int blocksize, currblock;

    if(!(ext2fd = ext2_get_file(fd))) {
        error_msg("Invalid file descriptor, cannot read!");
        return -1;
    }

    if((unsigned)(ext2fd->cursor + count) > ext2fd->inode->i_size) count -= (ext2fd->cursor + count) - ext2fd->inode->i_size;

    blocksize = ext2_get_blocksize();
    currblock = 0;
    offset = ext2fd->cursor;
    while(offset > blocksize) {
        offset -= blocksize;
        currblock++;
    }
    bytesread = 0;

    for(i = currblock; i < ext2fd->nblocks; ++i) {
        toread = (count >= blocksize) ? blocksize : count;
        count -= toread;
        bytesread += ext2_read_block(ext2fd->blocks[i], buf + bytesread, toread, offset);
        if(offset > 0) offset = 0;
        if(count == 0) break;
    }

    ext2fd->cursor += bytesread;

    return bytesread;
}
