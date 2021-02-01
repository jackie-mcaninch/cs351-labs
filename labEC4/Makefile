CC = gcc
CFLAGS = -Wall -pthread

build: membench
	
test: membench	
	./runbench.sh

membench: membench.c
	$(CC) $(CFLAGS) -o membench $<

clean:
	rm -rf membench
