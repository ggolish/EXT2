CC = gcc
OBJS = ext2open.o ext2read.o ext2write.o ext2seek.o ext2close.o utility.o ext2.o
ARGS = -Wall -g

ext2lib: $(OBJS)
	ar rcs ext2lib $?

test: test.c ext2lib ext2io.h
	gcc -o test test.c ext2lib

ext2.o: ext2.c ext2.h
	$(CC) $(ARGS) -c $<

ext2read.o: ext2read.c  ext2.h ext2io.h utility.h
	$(CC) $(ARGS) -c $<

ext2write.o: ext2write.c  ext2.h ext2io.h utility.h
	$(CC) $(ARGS) -c $<

ext2close.o: ext2close.c  ext2.h ext2io.h utility.h
	$(CC) $(ARGS) -c $<

ext2seek.o: ext2seek.c  ext2.h ext2io.h utility.h
	$(CC) $(ARGS) -c $<

ext2open.o: ext2open.c  ext2.h ext2io.h utility.h
	$(CC) $(ARGS) -c $<

utility.o: utility.c utility.h
	$(CC) $(ARGS) -c $<

clean:
	rm -f ext2lib test $(OBJS)

