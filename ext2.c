#include "ext2.h"
#include "utility.h"

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <math.h>

#define BYTE 8
#define KB 1024
#define SB_OFFSET 1024
#define EXT2_S_IFDIR 0x4000

// Global filesystem structure
static EXT2 *ext2fs = NULL;

static LLDIRLIST *make_lldirlist_node(LLDIR *ld);
static void lldirlist_insert(LLDIRLIST **head, LLDIR *ld);
static void get_blocks(int **blocks, int block, int indirection, int *n, int max);
static int get_block_from_inode(INODETABLE *it, int **blocks);

// Initializes the file system
void ext2_init(char *disk)
{
    unsigned int i;
    unsigned int offset;

    ext2fs = (EXT2 *)safe_malloc(sizeof(EXT2), 
            "Failed to allocate memory for file system!");
    ext2fs->sb = (SUPERBLOCK *)safe_malloc(sizeof(SUPERBLOCK), 
            "Failed to allocate memory for superblock!");

    // Open filesystem
    ext2fs->fd = safe_open(disk, O_APPEND, "Unable to open disk!");

    // Read superblock from filesystem
    lseek(ext2fs->fd, SB_OFFSET, SEEK_SET);
    safe_read(ext2fs->fd, ext2fs->sb, sizeof(SUPERBLOCK), "Failed to read from disk!");

    // Calculate block size for filesystem
    ext2fs->block_size = KB << ext2fs->sb->s_log_block_size;

    // Calculate number of block groups
    ext2fs->n_bg = ext2fs->sb->s_free_blocks_count / ext2fs->sb->s_blocks_per_group;
    if(ext2fs->n_bg == 0) ext2fs->n_bg++;
    ext2fs->bg = (BLOCKGROUP **)safe_malloc(ext2fs->n_bg * sizeof(BLOCKGROUP *), 
            "Failed to allocate memory for block group descriptor table!");

    // Read block group descriptors, start at third block for 1kb systems or
    // second block for larger block systems
    offset = (ext2fs->block_size == KB) ? 2 * KB : ext2fs->block_size;
    for(i = 0; i < ext2fs->n_bg; ++i) {
        ext2fs->bg[i] = (BLOCKGROUP *)safe_malloc(sizeof(BLOCKGROUP), 
                "Failed to allocate memory for block group descriptor!");
        lseek(ext2fs->fd, offset, SEEK_SET);
        read(ext2fs->fd, ext2fs->bg[i], sizeof(BLOCKGROUP));
        offset += sizeof(BLOCKGROUP);
    }

    // Initialize open files list
    ext2fs->maxfiles = 2;
    ext2fs->open_files = (EXT2_FILE **)safe_malloc(ext2fs->maxfiles * sizeof(EXT2_FILE *), "");
    ext2fs->nfiles = 0;
}

// Checks if file system has been initialized
int ext2checkfs()
{
    if(!ext2fs) return 0;
    return 1;
}

// closes the filesystem, frees memory
void ext2_close()
{
    unsigned int i;

    if(ext2fs->fd != -1)
        close(ext2fs->fd);
    free(ext2fs->sb);

    for(i = 0; i < ext2fs->n_bg; ++i)
        free(ext2fs->bg[i]);

    free(ext2fs->bg);

    free(ext2fs);
}

int ext2_insert_file(EXT2_FILE *ext2fd)
{
    int index;

    index = ext2fs->nfiles;
    ext2fs->open_files[ext2fs->nfiles++] = ext2fd;
    if(ext2fs->nfiles == ext2fs->maxfiles) {
        ext2fs->maxfiles *= 2;
        ext2fs->open_files = (EXT2_FILE **)safe_realloc(ext2fs->open_files, 
                sizeof(EXT2_FILE *) * ext2fs->maxfiles, "");
    }

    return index;
}

int ext2_delete_file(int index)
{
    EXT2_FILE *ext2fd;

    if(index >= ext2fs->nfiles) {
        error_msg("File cannot be closed, it wasn't opened!");
        return -1;
    }

    if(!(ext2fd = ext2fs->open_files[index])) {
        error_msg("File cannot be closed, it wasn't opened!");
        return -1;
    }   

    if(ext2fd->content) free(ext2fd->content);
    if(ext2fd->inode) free(ext2fd->inode);
    free(ext2fd);
    return 0;
}

LLDIRLIST *ext2_get_root()
{
    INODETABLE it;
    LLDIRLIST *rootdir;

    ext2_get_inode(&it, 2);
    rootdir = ext2_read_dir(&it);
    return rootdir;
}

LLDIRLIST *ext2_read_dir(INODETABLE *it)
{
    LLDIR *ld;
    LLDIRLIST *head = NULL;
    int *blocks;
    int n, i;
    unsigned int offset;

    // Check that the current inode is for a directory
    if(!(it->i_mode & EXT2_S_IFDIR)) {
        fprintf(stderr, "Inode found is not a directory!\n");
        exit(69);
    }

    n = get_block_from_inode(it, &blocks);

    // Iteratively read / store linked list directory structures
    offset = 0;
    ld = (LLDIR *)malloc(sizeof(LLDIR));
    for(i = 0; i < n; ++i) {
        while(offset < ext2fs->block_size) {
            lseek(ext2fs->fd, blocks[i] * ext2fs->block_size + offset, SEEK_SET);
            read(ext2fs->fd, ld, sizeof(LLDIR));
            lldirlist_insert(&head, ld);
            offset += ld->rec_len;
        }
        offset = 0;
    }

    free(ld);

    return head;
}

LLDIRLIST *ext2_read_subdir(LLDIRLIST *root, char *subdir, int type, INODETABLE *file)
{
    LLDIRLIST *ptr;
    LLDIRLIST *newdir = NULL;
    INODETABLE it;

    for(ptr = root; ptr; ptr = ptr->next) {
        if(memcmp(ptr->ld->name, subdir, ptr->ld->name_len) == 0) {
            switch(type) {
                case EXT2_FT_DIR:
                    ext2_get_inode(&it, ptr->ld->inode);
                    newdir = ext2_read_dir(&it);
                    return newdir;
                case EXT2_FT_REG_FILE:
                    ext2_get_inode(file, ptr->ld->inode);
                    return root;
                default:
                    error_msg("Invalid filetype!");
                    return NULL;
            }
        }
    }

    return newdir;
}

void ext2_print_lldirlist(LLDIRLIST *lldir)
{
    int i;

    if(!lldir) return;
    ext2_print_lldirlist(lldir->next);
    printf("%03d ", lldir->ld->inode);
    for(i = 0; i < lldir->ld->name_len; ++i)
        printf("%c", lldir->ld->name[i]);
    printf("\n");

}

void ext2_free_lldirlist(LLDIRLIST *t)
{
    if(!t) return;

    ext2_free_lldirlist(t->next);
    free(t->ld);
    free(t);
}

void ext2_get_inode(INODETABLE *it, int inode)
{
    inode--;
    lseek(ext2fs->fd, ext2fs->bg[0]->bg_inode_table * ext2fs->block_size + 
            (inode * ext2fs->sb->s_inode_size), SEEK_SET);
    safe_read(ext2fs->fd, it, ext2fs->sb->s_inode_size, "Unable to read inode!");
}

int ext2_read_inode_bitmap(int bgn, BITMAP *ibm)
{
    int nbytes;

    nbytes = ceil((float)ext2fs->sb->s_inodes_per_group / BYTE);
    (*ibm) = (BITMAP)malloc(nbytes);
    lseek(ext2fs->fd, (ext2fs->bg[bgn]->bg_inode_bitmap) * ext2fs->block_size, SEEK_SET);
    read(ext2fs->fd, (*ibm), nbytes);
    return nbytes;
}

static int get_block_from_inode(INODETABLE *it, int **blocks)
{
    int i, n, maxindex;

    int indir_list[] = {
        0, 0, 0, 0, 0,
        0, 0, 0, 0, 0,
        0, 0, 1, 2, 3
    };

    n = 0;
    maxindex = it->i_blocks / (2 << ext2fs->sb->s_log_block_size);
    (*blocks) = (int *)safe_malloc(maxindex * sizeof(int), "");
    for(i = 0; i < 15; ++i) {
        get_blocks(blocks, it->i_block[i], indir_list[i], &n, maxindex);
    }
    
    return n;
}

static void get_blocks(int **blocks, int block, int indirection, int *n, int max)
{
    unsigned int newblocks[ext2fs->block_size / 4], i;
    int ln;

    lseek(ext2fs->fd, ext2fs->block_size * block, SEEK_SET);
    if(indirection == 0) {
        ln = (*n);
        if(ln < max) {
            (*blocks)[ln++] = block;
        }
        (*n) = ln;
    } else {
        safe_read(ext2fs->fd, newblocks, ext2fs->block_size, "");
        for(i = 0; i < ext2fs->block_size / 4; ++i) {
            get_blocks(blocks, newblocks[i], indirection - 1, n, max);
            if((*n) >= max) break;
        }
    }
}

static LLDIRLIST *make_lldirlist_node(LLDIR *ld)
{
    LLDIRLIST *tmp;

    tmp = (LLDIRLIST *)malloc(sizeof(LLDIRLIST));
    tmp->ld = (LLDIR *)malloc(sizeof(LLDIR));
    memcpy(tmp->ld, ld, sizeof(LLDIR));
    tmp->next = NULL;
    return tmp;
}

static void lldirlist_insert(LLDIRLIST **head, LLDIR *ld)
{
    LLDIRLIST *new_node = make_lldirlist_node(ld);

    if((*head)) new_node->next = (*head);
    (*head) = new_node;
}

