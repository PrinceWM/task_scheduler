#include "thread_pool.h"
#include <stdio.h>
#include <stdlib.h>

#define TASK_NUM 5

void do_something(void *arg) {
    int n = *(int *)arg;
    printf("task #%d started\n", n);
    sleep(rand() % 5 + 1);
    printf("task #%d finished\n", n);
}

int main (int argc, char **argv)
{
    int i;
    int num[TASK_NUM];
    thread_pool_t *pool;

    pool = thread_pool_create(3);
    for(i = 0; i < TASK_NUM; ++i) {
        num[i] = i + 1;
        thread_pool_add_task(pool, do_something, &num[i]);
    }

    thread_pool_wait_for_done(pool);
    printf("all tasks are done\n");
    thread_pool_destroy(pool);

    return 0;
}
