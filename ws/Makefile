CC=gcc -O2	-Wall -g -pedantic

all:	main

main:	quicksort.o	sched.o	deque.o
	$(CC)	quicksort.o	sched.o	deque.o

quicksort.o:	quicksort.c	sched.h	deque.h
	$(CC)	-c	quicksort.c
sched.o:	sched.c	sched.h	deque.h
	$(CC)	-c	sched.c
deque.o:	deque.c	deque.h
	$(CC)	-c	deque.c
clean	:
	rm	-f	*.o	a.out	main