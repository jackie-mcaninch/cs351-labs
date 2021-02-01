#!/bin/bash

# iterate over the program parameters using nested for loops
# program usage: ./fileio <workload> <block_size> <num_procs>
#     - workload: WS (write-sequential), WR (write-random), RS (read sequential), RR (read random)
#     - block_size: 1KB, 10KB, 1000KB, 1MB
#     - num_procs: 1 2 4

# right now the bash script calls the program with only one configuration




# generate tests to measure throughput for tables 1a, 1b, 1c, and 1d, using a 1GB dataset

for w in 0 1 2					#determines workload (WR and RR both contained in 2)
do
	for c in 1 2 4 8 			#determines concurrency
	do
		for rs in 64 1000 16000		#determines record size (in kb)
		do	
			if [ $w = 2 ]
			then
				iozone "-i0" "-i2" "-t$c" "-s$(( 1000000/c ))" "-r$rs"
			elif [ $w = 1 ]
			then
				iozone "-i0" "-i1" "-t$c" "-s$(( 1000000/c ))" "-r$rs"
			else
				iozone "-i0" "-t$c" "-s$(( 1000000/c ))" "-r$rs"
			fi
		done
	done
done


# generate tests to measure latency for tables 2a and 2b, using a 4MB dataset

for c in 1 2 4 8				#determines concurrency
do
	iozone "-i0" "-i2" "-t$c" "-s$(( 4000/c ))" "-r4" "-O"
done
