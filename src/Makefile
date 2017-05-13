CC= clang
FAST=-05 -mtune=native -march=native -funroll-loops -m64
TEST=-pg -ftest-coverage -fprofile-arcs
CFLAGS=-Wall -Wextra -Wpointer-arith -Wunused -fpic -g


all: bitbuf.o
	$(CC) $(CFLAGS) -c bitbuf.c

test:
	$(CC) $(CFLAGS) bitbuf.o -o bb_test

bitbuf.o: bitbuf.c bitbuf.h
	$(CC) $(CFLAGS) -c bitbuf.c
