/*

Recursively read a file:

int indir = {0, 0, 0, ... , 0, 1, 2, 3}
for(i = 0; i < 15; ++i) {
    getblock(it.i_block[i], indir[i])
}

void getblock(int n, int levelindir)
{
    int blocks[blocksize / 4];

    if(levelindir == 0) {
        // You want this block
    } else {
        readblock();
        for(i = 0; i < blocksize / 4; ++i) {
            getblock(blocks[i], levelindir - 1);
        }
    }
}

// Also need to keep track of how much is read

*/
