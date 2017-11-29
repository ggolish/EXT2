#include "ext2.h"
#include "ext2io.h"
#include "utility.h"

#include <stdio.h>

#define BYTE 8

int ext2write(int fd, const char *buf, int count)
{
    EXT2_FILE *ext2fd;
    int endpos, offset, blocksize, currblock, blocksneeded, totalbytes;
    int towrite, written;
    int i;

    if(!(ext2fd = ext2_get_file(fd))) {
        error_msg("Invalid file descriptor, can't write!");
        return -1;
    }

    if(!(ext2fd->flags & EXT2_WRONLY)) {
        error_msg("Write flag not set, cannot write!");
        return -1;
    }

    endpos = ext2fd->cursor + count;
    blocksize = ext2_get_blocksize();
    blocksneeded = ext2fd->nblocks;
    totalbytes = blocksneeded * blocksize;
    while(totalbytes < endpos) {
        blocksneeded++;
        totalbytes += blocksize;
    }

    if(blocksneeded != ext2fd->nblocks) {
        ext2_add_blocks(ext2fd, blocksneeded - ext2fd->nblocks);
        return -1;
    }

    currblock = 0;
    for(offset = ext2fd->cursor; offset > blocksize; offset -= blocksize)
        currblock++;

    written = 0;
    for(i = currblock; i < ext2fd->nblocks; ++i) {
        towrite = (count >= blocksize) ? blocksize : count;
        count -= towrite;
        written += ext2_write_block(ext2fd->blocks[i], buf + written, towrite, (i == 0) ? offset : 0);
        if(count == 0) break;
    }

    ext2fd->cursor += written;

    // update inode if file size increases
    if((unsigned)ext2fd->cursor > ext2fd->inode->i_size) {
        ext2fd->inode->i_size = ext2fd->cursor; 
        ext2_update_inode(ext2fd->inode, ext2fd->iid);
    }

    return written;
}
