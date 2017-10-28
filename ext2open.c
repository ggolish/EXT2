#include "ext2io.h"
#include "utility.h"

#include <string.h>

#define SIZE 10

static char **split(const char *str, int *size);

EXT2_FILE *ext2open(EXT2 *fs, const char *pathname, int flags)
{
    EXT2_FILE *ext2fd;
    char **pieces;
    int len;

    ext2fd = (EXT2_FILE *)safe_malloc(sizeof(EXT2_FILE), 
            "Failed to allocate memory for ext2 file structure!");

    // Split path into pieces
    pieces = (char **)split(pathname, &len);

    return ext2fd;
}

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

