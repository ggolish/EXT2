#include "ext2.h"

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
#define BG_OFFSET 2048
#define EXT2_S_IFDIR 0x4000

static void error_msg(const char *err);
static LLDIRLIST *make_lldirlist_node(LLDIR *ld);
static void lldirlist_insert(LLDIRLIST **head, LLDIR *ld);

EXT2 *ext2_init(char *disk)
{
    EXT2 *fs;
    unsigned int i;
    unsigned int offset;

    if(!(fs = (EXT2 *)malloc(sizeof(EXT2)))) {
        error_msg("memory allocation error!");
        return NULL;
    }   

    fs->sb = (SUPERBLOCK *)malloc(sizeof(SUPERBLOCK));

    // Open filesystem
    if((fs->fd = open(disk, O_RDONLY)) == -1) {
        error_msg("Unable to open disk!");
        ext2_close(fs);
        return NULL;
    }

    // Read superblock from filesystem
    lseek(fs->fd, SB_OFFSET, SEEK_SET);
    if((read(fs->fd, fs->sb, sizeof(SUPERBLOCK))) <= 0) {
        error_msg("unable to read superblock!");
        ext2_close(fs);
        return NULL;
    }

    // Calculate block size for filesystem
    fs->block_size = KB << fs->sb->s_log_block_size;

    // Calculate number of block groups
    fs->n_bg = fs->sb->s_free_blocks_count / fs->sb->s_blocks_per_group;
    if(fs->n_bg == 0) fs->n_bg++;
    fs->bg = (BLOCKGROUP **)malloc(fs->n_bg * sizeof(BLOCKGROUP *));

    // Read block group descriptors
    offset = BG_OFFSET;
    for(i = 0; i < fs->n_bg; ++i) {
        fs->bg[i] = (BLOCKGROUP *)malloc(sizeof(BLOCKGROUP));
        lseek(fs->fd, offset, SEEK_SET);
        read(fs->fd, fs->bg[i], sizeof(BLOCKGROUP));
        offset += sizeof(BLOCKGROUP);
    }

    return fs;
}

void ext2_close(EXT2 *fs)
{
    unsigned int i;

    if(fs->fd != -1)
        close(fs->fd);
    free(fs->sb);

    for(i = 0; i < fs->n_bg; ++i)
        free(fs->bg[i]);

    free(fs->bg);

    free(fs);
}

LLDIRLIST *ext2_get_top_level(EXT2 *fs)
{
    INODETABLE it;
    LLDIR *ld = NULL;
    LLDIRLIST *head = NULL;
    unsigned int offset, i;

    // Read inode structure pointed to by first block group descriptor
    lseek(fs->fd, fs->bg[0]->bg_inode_table * fs->block_size + fs->sb->s_inode_size, SEEK_SET);

    if((read(fs->fd, &it, sizeof(INODETABLE))) <= 0) {
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
            while(offset < fs->block_size) {
                lseek(fs->fd, (it.i_block[i]) * fs->block_size + offset, SEEK_SET);
                read(fs->fd, ld, sizeof(LLDIR));
                lldirlist_insert(&head, ld);
                offset += ld->rec_len;
            }
            offset = 0;
        }
    }

    free(ld);

    return head;
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

int ext2_read_inode_bitmap(EXT2 *fs, int bgn, BITMAP *ibm)
{
    int nbytes;

    nbytes = ceil((float)fs->sb->s_inodes_per_group / BYTE);
    (*ibm) = (BITMAP)malloc(nbytes);
    lseek(fs->fd, (fs->bg[bgn]->bg_inode_bitmap) * fs->block_size, SEEK_SET);
    read(fs->fd, (*ibm), nbytes);
    return nbytes;
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
