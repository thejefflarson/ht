CFLAGS ?= -g
all: ht_test
ht.o: ht.c
ht_test: ht_test.o ht.o
clean:
	rm *.o