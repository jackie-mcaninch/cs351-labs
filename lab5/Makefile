CC=gcc
CFLAGS=-Wall -Ofast -g -pthread

build: cpubench

test-cpubench: cpubench
	./runbench.sh

cpubench: cpubench.c
	$(CC) $(CFLAGS) -o cpubench $<

clean:
	rm -rf cpubench

