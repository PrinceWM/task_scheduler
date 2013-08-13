#include "task_scheduler.h"
#include <stdio.h>
#include <stdlib.h>

#define TASK_NUM 5

class demo_task : public ff_task
{
public:
    demo_task(int n)
    {
        this->num = n;
    }

    ~demo_task()
    {
        printf("task #%d destroyed\n", num);
        fflush(stdout);
    }

    void run()
    {
        printf("task #%d started\n", num);
        fflush(stdout);
        sleep(rand() % 5 + 1);
        printf("task #%d finished\n", num);
        fflush(stdout);
    }

private:
    int num;
};

int main()
{
    int i;
    int done_num;
    task_scheduler ts(3);
    demo_task *task_list[TASK_NUM];

    srand(time(0));
    for(i = 0; i < TASK_NUM; ++i)
        ts.submit(task_list[i] = new demo_task(i));

    done_num = 0;
    while(1)
    {
        for(i = 0; i < TASK_NUM; ++i)
        {
            if(task_list[i] && ts.is_done(task_list[i]))
            {
                if(ts.checkout(task_list[i]))
                    delete task_list[i];
                task_list[i] = NULL;
                ++done_num;
            }
        }
        if(done_num == TASK_NUM)
        {
            printf("all tasks are done!\n");
            fflush(stdout);
            break;
        }
        usleep(500000);
    }
    return 0;
}
