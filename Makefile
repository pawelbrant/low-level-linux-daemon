OBJ = main.o local_functions.o

all: daemon
daemon: $(OBJ)
	gcc $(OBJ) -o daemon
$(OBJ): local_functions.h

.PHONY: clean
clean:
	rm -f *.o daemon
