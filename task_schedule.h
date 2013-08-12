#ifndef FF_THREAD_POOL_H
#define FF_THREAD_POOL_H

#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <pthread.h>
#include <list>
#include <algorithm>

class ff_task
{
public:
    ff_task();
    virtual ~ff_task();
    virtual void run() = 0;
};

class task_scheduler
{
public:
    task_scheduler(int thread_num);
    ~task_scheduler();
    bool is_done(ff_task *task);
    void submit(ff_task *task);
    bool checkout(ff_task *task);

public:
    void _run();

private:
    std::list<ff_task *> pending_list;
    std::list<ff_task *> done_list;
    pthread_mutex_t pending_mutex;
    pthread_mutex_t done_mutex;
    pthread_cond_t cond_ready;

    int shutdown; /*是否销毁线程池*/
    pthread_t *thread_id; /*线程池中所有线程的ID*/
    int thread_count; /*线程池中的线程数量*/
};

#endif
