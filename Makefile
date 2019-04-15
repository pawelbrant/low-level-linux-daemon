CC = gcc

all: main.c local_functions.o local_functions.h
	$(CC) main.c local_functions.o -o daemon

local_functions.o: local_functions.c local_functions.h
	$(CC) local_functions.c -c -o local_functions.o

clear:
	rm -f *.o
