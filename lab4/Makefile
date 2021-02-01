CC=gcc
CFLAGS=-Wall -O3

build: netio server

test-netio: netio
	./netio ...

netio: netio.c
	$(CC) $(CFLAGS)  -o netio $< 

server: socket-server.c
	$(CC) $(CFLAGS)  -o server $< 

clean:
	rm -rf netio

