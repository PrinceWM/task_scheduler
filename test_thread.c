#include "thread_pool.h"
#include <stdio.h>

#define TASK_NUM 8

void *two(void *arg) {
    int n = *(int *)arg;
    printf("thread %x arg = %d\n", pthread_self(), n);
}

int main (int argc, char **argv)
{
    int i;
    int num[TASK_NUM];
    thread_pool_t *pool;

    pool = thread_pool_create(8);
    for(i = 0; i < TASK_NUM; ++i) {
        num[i] = i + 1;
        thread_pool_add_task(pool, two, &num[i]);
    }

    thread_pool_wait_for_done(pool);
    thread_pool_destroy(pool);

    return 0;
}
