for size in 100000000 1000000000
do
	for threads in 1 2 4
	do
		for mode in vanilla mutex semaphore spinlock atomic
		do
			./syncbench $mode $size $threads
		done
	done
done
