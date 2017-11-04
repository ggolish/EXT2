#include "ext2.h"

int main()
{
    ext2open("/mysubdir/stuff", EXT2_READ);
    return 0;
}
