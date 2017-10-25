CC = gcc
OBJS = ext2global.o ext2open.o ext2read.o ext2write.o ext2seek.o ext2close.o
ARGS = -Wall -g
TARGET = ext2lib
LIBS = -lm

$(TARGET): $(OBJS)
	ar rcs ext2lib $?

ext2global.o: ext2global.c ext2.h
	$(CC) $(ARGS) -c $< $(LIBS)

ext2read.o: ext2read.c ext2.h
	$(CC) $(ARGS) -c $< $(LIBS)

ext2write.o: ext2write.c ext2.h
	$(CC) $(ARGS) -c $< $(LIBS)

ext2close.o: ext2close.c ext2.h
	$(CC) $(ARGS) -c $< $(LIBS)

ext2seek.o: ext2seek.c ext2.h
	$(CC) $(ARGS) -c $< $(LIBS)

ext2open.o: ext2open.c ext2.h
	$(CC) $(ARGS) -c $< $(LIBS)

clean:
	rm -f $(TARGET) $(OBJS)

