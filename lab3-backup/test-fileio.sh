#!/bin/bash

# iterate over the program parameters using nested for loops
# program usage: ./fileio <workload> <block_size> <num_procs>
#     - workload: WS (write-sequential), WR (write-random), RS (read sequential), RR (read random)
#     - block_size: 1KB, 10KB, 1000KB, 1MB
#     - num_procs: 1 2 4

# right now the bash script calls the program with only one configuration


# perform test on 4MB dataset

for w in WR RR					#determines workload
do
	for c in 1 2 4 8			#determines concurrency
	do
		if [ $c -eq 1 ]
		then
			FILES="1-1.txt"
		fi
		if [ $c -eq 2 ]
		then
			FILES="2-1.txt 2-2.txt"
		fi
		if [ $c -eq 4 ]
		then
			FILES="3-1.txt 3-2.txt 3-3.txt 3-4.txt"

		fi
		if [ $c -eq 8 ]
		then
			FILES="4-1.txt 4-2.txt 4-3.txt 4-4.txt 4-5.txt 4-6.txt 4-7.txt 4-8.txt"

		fi
		./fileio $w 4 $c $FILES

	done
done


# perform test on 1GB dataset

for w in WS RS WR RR				#determines workload
do
	for c in 1 2 4 8 			#determines concurrency
	do
		if [ $c -eq 1 ]
		then
			FILES="5-1.txt"
		fi

		if [ $c -eq 2 ]
		then
			FILES="6-1.txt 6-2.txt"
		fi

		if [ $c -eq 4 ]
		then
			FILES="7-1.txt 7-2.txt 7-3.txt 7-4.txt"
		fi

		if [ $c -eq 8 ]
		then
			FILES="8-1.txt 8-2.txt 8-3.txt 8-4.txt 8-5.txt 8-6.txt 8-7.txt 8-8.txt"
		fi
		
		for rs in 64 1024 16384		#determines record size (in kb)
		do	
			./fileio $w $rs $c $FILES
		done
	done
done
