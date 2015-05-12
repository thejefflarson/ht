CFLAGS ?= -g -std=c99
all: ht_test
ht.o: ht.c
ht_test: ht_test.o ht.o
clean:
	rm -r *.o ht_test ht_test.dSYM