CC=gcc
CFLAGS=-c -w


all: man clean

man: man.o linkedlist.o emalloc.o
	$(CC) man.o linkedlist.o emalloc.o -o man -lreadline

man.o: man.c linkedlist.h emalloc.h
	$(CC) $(CFLAGS) man.c

linkedlist.o: linkedlist.c linkedlist.h emalloc.h
	$(CC) $(CFLAGS) linkedlist.c

emalloc.o: emalloc.c emalloc.h
	$(CC) $(CFLAGS) emalloc.c

clean:
	rm -rf *.o 
