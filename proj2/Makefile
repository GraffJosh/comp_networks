CC=gcc
CFLAGS=-g
DEPS = project2.h
OBJ = project2.o student2.o a_fsm.o b_fsm.o

%.o: %.c $(DEPS)
	reset
	$(CC) -c -o $@ $< $(CFLAGS)

project2: $(OBJ)
	gcc -o $@ $^ $(CFLAGS)

clean:
	-rm -f *.o
	-rm -f project2