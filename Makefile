CC = gcc

CFLAGS = -Wall -g -pedantic

.PHONY: all clean lifo ws run

all: lifo/lifo ws/ws

lifo: lifo/lifo

ws: ws/ws

lifo/lifo: lifo/quicksort.o lifo/sched.o lifo/pile.o
	$(CC) $(CFLAGS) $^ -o $@

ws/ws: ws/quicksort.o ws/sched.o ws/deque.o
	$(CC) $(CFLAGS) $^ -O2 -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -O2 -o $@

clean:
	rm -f lifo/*.o ws/*.o lifo/lifo ws/ws


