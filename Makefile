pool: thread_pool.h thread_pool.c test_thread.c
	gcc -O2 -lpthread -o pool thread_pool.c test_thread.c

clean:
	rm -rf pool
