#include "task_scheduler.h"

ff_task::ff_task()
{
}

ff_task::~ff_task()
{
}

static void *thread_routine(void *arg)
{
    task_scheduler *ts = (task_scheduler *)arg;
    ts->_run();
    return NULL;
}

/**
 * thread_num: 线程池中线程的数量
 */
task_scheduler::task_scheduler(int thread_num)
{
    pthread_mutex_init(&this->pending_mutex, NULL);
    pthread_mutex_init(&this->done_mutex, NULL);
    pthread_cond_init(&this->cond_ready, NULL);

    this->shutdown = 0;
    this->thread_count = thread_num;
    this->thread_id = new pthread_t[thread_num];

    for(int i = 0; i < thread_num; ++i)
        pthread_create(&this->thread_id[i], NULL, thread_routine, this);
}

void task_scheduler::_run()
{
    ff_task *task;
    while(1)
    {
        pthread_mutex_lock(&this->pending_mutex);
        while(this->pending_list.size() == 0 && !this->shutdown)
            pthread_cond_wait(&this->cond_ready, &this->pending_mutex);

        if(this->shutdown)
        {
            pthread_mutex_unlock(&this->pending_mutex);
            pthread_exit(NULL);
        }

        task = this->pending_list.front();
        this->pending_list.pop_front();
        pthread_mutex_unlock(&this->pending_mutex);

        task->run();

        pthread_mutex_lock(&this->done_mutex);
        this->done_list.push_back(task);
        pthread_mutex_unlock(&this->done_mutex);
    }
}

bool task_scheduler::is_done(ff_task *task)
{
    bool done;
    std::list<ff_task *>::iterator iter;

    pthread_mutex_lock(&this->done_mutex);
    iter = std::find(this->done_list.begin(), this->done_list.end(), task);
    done = !(iter == this->done_list.end());
    pthread_mutex_unlock(&this->done_mutex);
    return done;
}

bool task_scheduler::checkout(ff_task *task)
{
    bool done;
    std::list<ff_task *>::iterator iter;

    pthread_mutex_lock(&this->done_mutex);
    iter = std::find(this->done_list.begin(), this->done_list.end(), task);
    done = !(iter == this->done_list.end());
    if(done)
        this->done_list.erase(iter);
    pthread_mutex_unlock(&this->done_mutex);
    return done;
}

/**
 * 功能：添加一个任务并通知线程池中空闲的线程去执行
 * 注意：不能将已提交的任务再次提交，除非它已经完成
 */
void task_scheduler::submit(ff_task *task)
{
    pthread_mutex_lock(&this->pending_mutex);
    this->pending_list.push_back(task);
    pthread_mutex_unlock(&this->pending_mutex);

    pthread_cond_signal(&this->cond_ready);
}

/**
 * 功能：通知并等待线程池中的线程退出，然后释放线程池占用的所有资源
 */
task_scheduler::~task_scheduler()
{
    std::list<ff_task *>::iterator iter;

    pthread_mutex_lock(&this->pending_mutex);
    this->shutdown = 1;
    pthread_mutex_unlock(&this->pending_mutex);

    pthread_cond_broadcast(&this->cond_ready);
    for(int i = 0; i < this->thread_count; ++i)
        pthread_join(this->thread_id[i], NULL);

    delete[] this->thread_id;

    for(iter = this->pending_list.begin(); iter != this->pending_list.end(); ++iter)
        delete *iter;
    for(iter = this->done_list.begin(); iter != this->done_list.end(); ++iter)
        delete *iter;

    pthread_mutex_destroy(&this->pending_mutex);
    pthread_mutex_destroy(&this->done_mutex);
    pthread_cond_destroy(&this->cond_ready);
}
