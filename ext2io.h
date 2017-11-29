#ifndef EXT2IO_H_
#define EXT2IO_H_

#define EXT2_RDONLY     0x1
#define EXT2_WRONLY     0x2
#define EXT2_RDWR       0x3
#define EXT2_CREAT      0x4
#define EXT2_TRUNC      0x8

#define EXT2_SEEK_SET   0x1
#define EXT2_SEEK_CUR   0x2
#define EXT2_SEEK_END   0x4

extern int ext2open(const char *pathname, int flags);
extern int ext2close(int fd);
extern int ext2read(int fd, char *buf, int count);
extern int ext2seek(int fd, int offset, int whence);
extern int ext2write(int fd, const char *buf, int count);

#endif
