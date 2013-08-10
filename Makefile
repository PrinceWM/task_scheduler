pool: thread_pool.h thread_pool.c test_thread.c
	gcc -O2 -Wall -o pool thread_pool.c test_thread.c -pthread

clean:
	rm -rf pool
