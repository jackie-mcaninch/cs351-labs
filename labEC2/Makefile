CC = gcc
CFLAGS = -Wall -pthread

build: syncbench

test: syncbench
	./runbench.sh

syncbench: syncbench.c
	$(CC) $(CFLAGS) -o syncbench $<

clean:
	rm -rf syncbench
