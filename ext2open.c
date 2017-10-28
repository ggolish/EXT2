#include "ext2io.h"
#include "utility.h"

EXT2_FILE *ext2open(EXT2 *fs, const char *pathname, int flags)
{
    EXT2_FILE *ext2fd;

    ext2fd = (EXT2_FILE *)safe_malloc(sizeof(EXT2_FILE));
    return ext2fd;
}


