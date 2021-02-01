#!/bin/bash
COUNTER=0
for a in 1 2 4 8
do
	COUNTER=$((COUNTER+1))
	for b in $(seq 1 $a)
	do
		head /dev/urandom -c $((4096/$a))K > "$COUNTER-$b.txt"
	done
done

COUNTER=0;
for a in 1 2 4 8
do
	COUNTER=$((COUNTER+1));
	for b in $(seq 1 $a)
	do
		head /dev/urandom -c $((1024/$a))M > "$(($COUNTER+4))-$b.txt"
	done
done
