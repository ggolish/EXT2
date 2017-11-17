#include "ext2.h"
#include "utility.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIZE 10

static char **split(const char *str, int *size);

int ext2open(const char *pathname, int flags)
{
    EXT2_FILE *ext2fd;
    LLDIRLIST *current_dir;
    char **pieces;
    char *env_ptr;
    int len, i, fd;

    // Initialize filesystem if needed
    if(!ext2checkfs()) {
        env_ptr = safe_getenv("EXT2_IMAGE_PATH");
        ext2_init(env_ptr);
    }

    ext2fd = (EXT2_FILE *)safe_malloc(sizeof(EXT2_FILE), 
            "Failed to allocate memory for ext2 file structure!");

    ext2fd->inode = (INODETABLE *)safe_malloc(sizeof(INODETABLE), "");
    ext2fd->flags = flags;
    ext2fd->content = NULL;
    ext2fd->cursor = 0;

    // Split path into pieces
    pieces = (char **)split(pathname, &len);

    // Read root directory
    current_dir = ext2_get_root();

    for(i = 0; i < len - 1; ++i) {
        current_dir = ext2_read_subdir(current_dir, pieces[i], EXT2_FT_DIR, NULL);
        if(!current_dir) {
            error_msg("Unable to open directory!");
            return -1;
        }
    }

    if(!ext2_read_subdir(current_dir, pieces[len - 1], EXT2_FT_REG_FILE, ext2fd->inode)) {
        error_msg("Unable to open file!");
        return -1;
    }

    fd = ext2_insert_file(ext2fd);

    // Free path memory
    for(i = 0; i < len; ++i) free(pieces[i]);
    free(pieces);

    return fd;
}

// Helper function that splits a string based on the character '/' and returns
// an array of strings
static char **split(const char *str, int *size)
{
    char tmp[strlen(str)];
    char *s, *t;
    char **list;
    int count, maxsize;

    strcpy(tmp, str);
    s = tmp;
    count = 0;
    list = (char **)safe_malloc(SIZE * sizeof(char *), "Allocation failed!");
    maxsize = SIZE;
    while((t = strchr(s, '/'))) {
        *t = '\0';
        if(strlen(s) > 0)
            list[count++] = strdup(s);
        if(count == maxsize - 1) {
            maxsize *= 2;
            list = (char **)safe_realloc(list, maxsize * sizeof(char *), 
                    "Memory reallocation failed during split!");
        }
        s = t + 1;
    }

    if(strlen(s) > 0)
        list[count++] = strdup(s);

    *size = count;
    return list;
}

