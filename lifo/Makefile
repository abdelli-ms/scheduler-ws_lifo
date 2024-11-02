CC=gcc	-Wall	-g	-pedantic

all:	main

main:	quicksort.o	sched.o	pile.o
	$(CC)	-O2	quicksort.o	sched.o	pile.o

quicksort.o	:	quicksort.c	pile.h	sched.h
	$(CC)	-c	quicksort.c

sched.o:	sched.c	sched.h	pile.h

pile.o:	pile.c	pile.h	sched.h
	$(CC)	-c	pile.c

clean	:
	rm	-f	*.o	a.out	main
