#ifndef EXT2_H_
#define EXT2_H_

typedef struct __attribute__((packed)) {
    unsigned int s_inodes_count;
    unsigned int s_blocks_count;
    unsigned int s_r_blocks_count;
    unsigned int s_free_blocks_count;
    unsigned int s_free_inodes_count;
    unsigned int s_first_data_block;
    unsigned int s_log_block_size;
    unsigned int s_log_frag_size;
    unsigned int s_blocks_per_group;
    unsigned int s_frags_per_group;
    unsigned int s_inodes_per_group;
    unsigned int s_mtime;
    unsigned int s_wtime;
    unsigned short s_mnt_count;
    unsigned short s_max_mnt_count;
    unsigned short s_magic;
    unsigned short s_state;
    unsigned short s_errors;
    unsigned short s_minor_rev_level;
    unsigned int s_lastcheck;
    unsigned int s_checkinterval;
    unsigned int s_creator_os;
    unsigned int s_rev_level;
    unsigned short s_def_resuid;
    unsigned short s_def_resgid;
    unsigned int s_first_ino;
    unsigned short s_inode_size;
    unsigned short s_block_group_nr;
    unsigned int s_feature_compat;
    unsigned int s_feature_incompat;
    unsigned int s_feature_ro_compat;
    char s_uuid[16];
    char s_volume_name[16];
    char s_last_mounted[64];
    char unused[824];
} SUPERBLOCK;

typedef struct __attribute__((packed)) {
    unsigned int bg_block_bitmap;
    unsigned int bg_inode_bitmap;
    unsigned int bg_inode_table;
    unsigned short bg_free_blocks_count;
    unsigned short bg_free_inodes_count;
    unsigned short bg_used_dirs_count;
    unsigned short bg_pad;
    char bg_reserved[12];
} BLOCKGROUP;

typedef struct __attribute__((packed)) {
    unsigned short i_mode;
    unsigned short i_uid;
    unsigned int i_size;
    unsigned int i_atime;
    unsigned int i_ctime;
    unsigned int i_mtime;
    unsigned int i_dtime;
    unsigned short i_gid;
    unsigned short i_links_count;
    unsigned int i_blocks;
    unsigned int i_flags;
    unsigned int i_osd1;
    unsigned int i_block[15];
    unsigned int i_generation;
    unsigned int i_file_acl;
    unsigned int i_dir_acl;
    unsigned int i_faddr;
    char i_osd2[12];
} INODETABLE;

#define EXT2_FT_REG_FILE    1
#define EXT2_FT_DIR         2

typedef struct __attribute__((packed)) {
    unsigned int inode;
    unsigned short rec_len;
    char name_len;
    char file_type;
    char name[255];
} LLDIR;

typedef struct node {
    LLDIR *ld;
    struct node *next;
} LLDIRLIST;

typedef struct {
    INODETABLE *inode;
    char **content;
    int flags;
    int *blocks;
    int nblocks;
    int cursor;
} EXT2_FILE;

typedef struct {
    SUPERBLOCK *sb;
    BLOCKGROUP **bg;
    EXT2_FILE **open_files;
    int nfiles;
    int maxfiles;
    int fd;
    unsigned int block_size;
    unsigned int n_bg;
} EXT2;

typedef
    unsigned char *
BITMAP;

typedef 
    unsigned char
BITMASK;

extern void ext2_init(char *disk);
extern void ext2_close();

extern int ext2_get_blocksize();

extern int ext2_insert_file(EXT2_FILE *ext2fd);
extern int ext2_delete_file(int index);
extern EXT2_FILE *ext2_get_file(int index);

extern int ext2_read_block(int blockid, char *buf, int count, int offset);

extern void ext2_get_inode(INODETABLE *it, int inode);
extern int ext2_read_inode_bitmap(int bgn, BITMAP *ibm);

extern LLDIRLIST *ext2_get_root();
extern LLDIRLIST *ext2_read_dir(INODETABLE *it);
extern void ext2_free_lldirlist(LLDIRLIST *t);
extern void ext2_print_lldirlist(LLDIRLIST *lldir);
extern LLDIRLIST *ext2_read_subdir(LLDIRLIST *root, char *subdir, int type, INODETABLE *file);

extern int ext2checkfs();

#define EXT2_READ   0x1
#define EXT2_WRITE  0x2

extern int ext2open(const char *pathname, int flags);
extern int ext2close(int fd);
int ext2read(int fd, char *buf, int count);

#endif
