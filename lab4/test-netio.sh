#!/bin/bash
for op in add subtract multiply divide
do
	for funct in function pipe socket
	do
		if [ "$funct" = "function" ]
		then
			./netio $funct $op 1000000000
		else
			./netio $funct $op 1000000
		fi
	done
done
