default: client server

client.o: client.c
	gcc -c client.c -o client.o

client: client.o
	gcc client.o -o client

server.o: server.c
	gcc -c server.c -o server.o

server: server.o
	gcc server.o -o server

clean:
	-rm -f *.o
	-rm -f client server