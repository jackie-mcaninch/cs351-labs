#!/bin/bash

for m in flops
do
	for t in single double
	do
		for size in 10 100 1000
		do
			for threads in 1 2 4
			do
				./cpubench $m $t $size $threads
			done
		done
	done
done

