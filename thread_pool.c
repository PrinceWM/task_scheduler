#include "thread_pool.h"

void *thread_routine(void *arg);

/**
 * 功能：创建线程池
 * thread_num: 线程池中线程的数量
 * 返回值：创建成功则返回指向已创建的的线程池的指针，
 * 否则，返回NULL
 */
thread_pool_t *thread_pool_create(int thread_num) {
    int i;
    thread_pool_t *pool;
    pool = (thread_pool_t *)malloc(sizeof(thread_pool_t));

    pthread_mutex_init(&pool->queue_mutex, NULL);
    pthread_cond_init(&pool->queue_cond_ready, NULL);

    pool->queue_head = NULL;
    pool->queue_tail = NULL;
    pool->cur_task_count = 0;
    pool->shutdown = 0;
    pool->thread_count = thread_num;
    pool->thread_id = (pthread_t *) malloc(thread_num * sizeof(pthread_t));

    for(i = 0; i < thread_num; ++i)
        pthread_create(&pool->thread_id[i], NULL, thread_routine, pool);

    return pool;
}

/**
 * 功能：添加一个任务并通知线程池中空闲的线程去执行
 * pool: 指向一个已经初始化的线程池
 * task: 函数指针，要执行的任务
 * arg: 传递给函数 task 的参数
 * 返回值：无
 */
void thread_pool_add_task(thread_pool_t *pool, void (*task_func)(void *), void *arg) {
    thread_task_t *task = (thread_task_t *)malloc(sizeof(thread_task_t));
    task->task_func = task_func;
    task->arg = arg;
    task->link = NULL;

    pthread_mutex_lock(&pool->queue_mutex);
    if(pool->queue_head != NULL) {
        pool->queue_tail->link = task;
        pool->queue_tail = task;
    }
    else
        pool->queue_head = pool->queue_tail = task;
    pool->cur_task_count++;

    pthread_mutex_unlock(&pool->queue_mutex);
    pthread_cond_signal(&pool->queue_cond_ready);
}

/**
 * 功能：获取当前线程池中的任务数量
 * pool: 指向一个已经初始化的线程池
 */
int thread_pool_get_current_task_count(thread_pool_t *pool)
{
    int count;
    pthread_mutex_lock(&pool->queue_mutex);
    count = pool->cur_task_count;
    pthread_mutex_unlock(&pool->queue_mutex);
    return count;
}

/**
 * 功能：等待线程池中的任务全部完成
 * pool: 指向一个已经初始化的线程池
 */
void thread_pool_wait_for_done(thread_pool_t *pool)
{
    while(thread_pool_get_current_task_count(pool) > 0)
        usleep(100000);
}

/**
 * 功能：通知并等待线程池中的线程退出，然后释放线程池占用的所有资源
 * pool: 指向要销毁的线程池（销毁后不可再引用pool所指向的内存）
 * 返回值：无
 */
void thread_pool_destroy(thread_pool_t *pool) {
    thread_task_t *task;
    int i;
    if(pool->shutdown)
        return;
    pthread_mutex_lock(&pool->queue_mutex);
    pool->shutdown = 1;
    pthread_mutex_unlock(&pool->queue_mutex);

    pthread_cond_broadcast(&pool->queue_cond_ready);
    for(i = 0; i < pool->thread_count; ++i)
        pthread_join(pool->thread_id[i], NULL);

    free(pool->thread_id);
    while(pool->queue_head != NULL) {
        task = pool->queue_head;
        pool->queue_head = task->link;
        free(task);
    }
    pthread_mutex_destroy(&pool->queue_mutex);
    pthread_cond_destroy(&pool->queue_cond_ready);
    free(pool);
}

/**
 * 功能：取出任务列表中的任务并执行
 * arg: 指向当前线程所属的线程池
 * 返回值：NULL
 */
void *thread_routine(void *arg) {
    thread_pool_t *pool;
    thread_task_t *task;
    pool = (thread_pool_t *)arg;
    while(1) {
        pthread_mutex_lock(&pool->queue_mutex);
        while(pool->cur_task_count == 0 && !pool->shutdown)
            pthread_cond_wait(&pool->queue_cond_ready, &pool->queue_mutex);

        if(pool->shutdown){
            pthread_mutex_unlock(&pool->queue_mutex);
            pthread_exit(NULL);
        }

        pool->cur_task_count--;
        task = pool->queue_head;
        pool->queue_head = task->link;
        pthread_mutex_unlock(&pool->queue_mutex);

        (*task->task_func)(task->arg);
        free(task);
        task = NULL;
    }
}
