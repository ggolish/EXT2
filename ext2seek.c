#include "ext2.h"
#include "utility.h"

int ext2seek(int fd, int offset, int whence)
{
    EXT2_FILE *ext2fd;

    if(!(ext2fd = ext2_get_file(fd))) return -1;

    switch(whence) {
        case EXT2_SEEK_SET:
            if(offset > 0 && (unsigned)offset < ext2fd->inode->i_size)
                ext2fd->cursor = offset;
            else return -1;
            break;
        case EXT2_SEEK_CUR:
            if((unsigned)ext2fd->cursor + offset < ext2fd->inode->i_size)
                ext2fd->cursor += offset;
            else return -1;
            break;
        case EXT2_SEEK_END:
            if(ext2fd->inode->i_size - offset > 0)
                ext2fd->cursor = ext2fd->inode->i_size - offset;
            else return -1;
            break;
        default:
            return -1;
    }

    return ext2fd->cursor;
}
