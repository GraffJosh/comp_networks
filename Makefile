HEADERS = client.h 

default: client

client.o: client.c ${HEADERS}
	gcc -c client.c -o client.o

client: client.o
	gcc client.o -o client

clean:
	-rm -f *.o
	-rm -f client