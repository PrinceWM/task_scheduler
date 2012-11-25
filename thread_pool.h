#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <pthread.h>

typedef struct tp_task {
    void *(*task_func)(void *arg);
    void *arg;
    struct tp_task *link;
}thread_task_t;

typedef struct {
    pthread_mutex_t queue_mutex;
    pthread_cond_t queue_cond_ready;

    thread_task_t *queue_head; /*线程池中的任务列表表头*/
    thread_task_t *queue_tail; /*线程池中的任务列表表尾*/
    int cur_task_count; /*当前队列中的任务数量*/

    int shutdown; /*是否销毁线程池*/
    pthread_t *thread_id; /*线程池中所有线程的ID*/
    int thread_count; /*线程池中的线程数量*/
}thread_pool_t;

thread_pool_t *thread_pool_create(int thread_num);
void thread_pool_add_task(thread_pool_t *pool, void *(*task_func)(void *), void *arg);
void thread_pool_destroy(thread_pool_t *pool);