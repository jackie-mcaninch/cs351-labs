CC=gcc
CFLAGS=-Wall

build: fileio
	
full: create_datasets.sh fileio test-fileio.sh test-iozone.sh
	./create_datasets.sh
	./test-fileio.sh
	./test-iozone.sh
	rm *.txt
test-fileio: create_datasets.sh fileio test-fileio.sh
	./create_datasets.sh
	./test-fileio.sh
	rm *.txt

test-iozone: test-iozone.sh
	./test-iozone.sh

fileio: fileio.c
	$(CC) $(CFLAGS) -o fileio $<

clean:
	rm -rf fileio

