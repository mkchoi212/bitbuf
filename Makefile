CC=gcc
DEBUG=-g -gdwarf-3
WARN=-Wno-shift-negative-value -Wall -Wextra -Wpointer-arith -Wunused
OP=-O3 -march=native -mtune=native -funroll-loops -m64
TST=--coverage -pg
RM=rm -f

SRC=bitbuf.c
TRASH=*.o *.out *.gc*

.PHONY: bitbuf.o

all: libbitbuf.a

libbitbuf.a: format bitbuf.o
	ar -rcus libbitbuf.a bitbuf.o

test:
	$(CC) $(WARN) $(DEBUG) $(TST) bitbuf_test.c bitbuf.c -o bb_test
	./bb_test

ftest: test
	make valgrind

valgrind:
	valgrind --leak-check=full --error-exitcode=42 ./bb_test

stat:
	@make test >/dev/null
	@command -v gcov >/dev/null 2>&1 || { echo >&2 "No gcov: skipping coverage test"; exit 1; }
	@command -v gprof >/dev/null 2>&1 || { echo >&2 "No gprof: skipping performance test"; exit 1; }
	gcov bitbuf.c
	gprof bb_test > profile.txt

format:
	clang-format -i --style=Google bitbuf.[ch] bitbuf_test.c


bitbuf.o: bitbuf.h bitbuf.c
	$(CC) $(WARN) $(OP) -fPIC -c bitbuf.c

clean:
	$(RM) $(TRASH)
	
