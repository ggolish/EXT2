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

static void error_msg(const char *err);
static LLDIRLIST *make_lldirlist_node(LLDIR *ld);
static void lldirlist_insert(LLDIRLIST **head, LLDIR *ld);
static LLDIR *find_dir_by_name(LLDIRLIST *dir, char *name);

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
    ext2fs->open_files = NULL;
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

LLDIRLIST *ext2_get_root()
{
    INODETABLE it;
    LLDIR *ld = NULL;
    LLDIRLIST *head = NULL;
    unsigned int offset, i;

    // Read inode structure pointed to by first block group descriptor
    lseek(ext2fs->fd, ext2fs->bg[0]->bg_inode_table * ext2fs->block_size + ext2fs->sb->s_inode_size, SEEK_SET);

    if((read(ext2fs->fd, &it, sizeof(INODETABLE))) <= 0) {
        fprintf(stderr, "Unable to read first inode table!\n");
        exit(69);
    }

    // Check that the current inode is for a directory
    if(!(it.i_mode & EXT2_S_IFDIR)) {
        fprintf(stderr, "Inode found is not a directory!\n");
        exit(69);
    }

    // Iteratively read / store linked list directory structures
    offset = 0;
    ld = (LLDIR *)malloc(sizeof(LLDIR));
    for(i = 0; i < it.i_blocks; ++i) {
        if(it.i_block[i] != 0) {
            while(offset < ext2fs->block_size) {
                lseek(ext2fs->fd, (it.i_block[i]) * ext2fs->block_size + offset, SEEK_SET);
                read(ext2fs->fd, ld, sizeof(LLDIR));
                lldirlist_insert(&head, ld);
                offset += ld->rec_len;
            }
            offset = 0;
        }
    }

    free(ld);

    return head;
}

LLDIRLIST *ext2_read_subdir(LLDIRLIST *root, char *subdir)
{
    LLDIRLIST *ptr;

    for(ptr = root; ptr; ptr = ptr->next) {
        if(memcmp(ptr->ld->name, subdir, ptr->ld->name_len) == 0) {
            printf("Found subdir: %s\n", subdir);
        }
    }

    return ptr;
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

int ext2_read_inode_bitmap(int bgn, BITMAP *ibm)
{
    int nbytes;

    nbytes = ceil((float)ext2fs->sb->s_inodes_per_group / BYTE);
    (*ibm) = (BITMAP)malloc(nbytes);
    lseek(ext2fs->fd, (ext2fs->bg[bgn]->bg_inode_bitmap) * ext2fs->block_size, SEEK_SET);
    read(ext2fs->fd, (*ibm), nbytes);
    return nbytes;
}

INODETABLE *ext2_read_inode(LLDIRLIST *dir, char **path, int index, int len)
{
    if(index == len) return NULL;
    
    LLDIR *ld;

    if(!(ld = find_dir_by_name(dir, path[index]))) return NULL;
    return NULL;
}

static LLDIR *find_dir_by_name(LLDIRLIST *dir, char *name)
{ 
    if(!dir) return NULL;

    dir->ld->name[dir->ld->name_len + 1] = '\0';
    if(strcmp(name, dir->ld->name) == 0) return dir->ld;
    return find_dir_by_name(dir->next, name);
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

static void error_msg(const char *err)
{
    fprintf(stderr, "\033[31m%s\033[0m\n", err);
}
