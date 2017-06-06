CC=clang
DEBUG=-g -gdwarf-3
WARN=-Wall -Wextra -Wpointer-arith -Wunused
OP=-O3 -march=native -mtune=native -funroll-loops -m64
TST=-pg -ftest-coverage -fprofile-arcs
RM=rm -f

SRC=bitbuf.c
TRASH=*.o *.out *.gc*

.PHONY: bitbuf.o

all: libbitbuf.a

libbitbuf.a: bitbuf.o
	ar -rcus libbitbuf.a bitbuf.o

test:
	$(CC) $(WARN) $(DEBUG) bitbuf_test.c bitbuf.c -o bb_test
	./bb_test
ftest:
	$(CC) $(WARN) $(DEBUG) $(OP) bitbuf_test.c.c bitbuf.c -o bb_test
	time ./bb_test
ttest:
	$(CC) $(WARN) $(TST) $(OP) bitbuf_test.c.c bitbuf.c -o bb_test
	./bb_test
	gcov bitbuf.gcda
	gprof bb_test > bitbuf.gprof

bitbuf.o: bitbuf.h bitbuf.c
	$(CC) $(WARN) $(OP) -fPIC -c bitbuf.c

clean:
	$(RM) $(TRASH)
	
