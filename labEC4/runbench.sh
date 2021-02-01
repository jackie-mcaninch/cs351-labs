#!/bin/bash
for size in 4000 4000000 2000000000
do
	for op in memset memcpy my_memset my_memcpy
	do
		./membench $op $size
	done
done

