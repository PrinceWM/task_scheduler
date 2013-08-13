#include "task_scheduler.h"
#include <stdio.h>

#define TASK_NUM 5

class cpi : public ff_task
{
public:
    cpi(size_t id, size_t n)
    {
        this->id = id;
        this->n = n;
    }

    ~cpi()
    {
        printf("task #%d destroyed\n", id);
        fflush(stdout);
    }

    double get_partial_pi() const
    {
        return this->partial_pi;
    }

    void run()
    {
        double h;
        double sum;
        double x;

        h   = 1.0 / (double) n;
        sum = 0.0;
        for (size_t i = id + 1; i <= n; i += TASK_NUM)
        {
            x = h * ((double)i - 0.5);
            sum += f(x);
        }
        partial_pi = h * sum;
    }

private:
    double f(double x)
    {
        return (4.0 / (1.0 + x * x));
    }

private:
    size_t n;
    size_t id;
    double partial_pi;
};

int main()
{
    int done_num;
    task_scheduler ts(3);
    cpi *task_list[TASK_NUM];
    double pi;
    size_t i;
    size_t n = 12345678;

    for(i = 0; i < TASK_NUM; ++i)
        ts.submit(task_list[i] = new cpi(i, n));

    done_num = 0;
    pi = 0.0;
    while(1)
    {
        for(i = 0; i < TASK_NUM; ++i)
        {
            if(task_list[i] && ts.is_done(task_list[i]))
            {
                if(ts.checkout(task_list[i]))
                {
                    pi += task_list[i]->get_partial_pi();
                    delete task_list[i];
                }
                task_list[i] = NULL;
                ++done_num;
            }
        }
        if(done_num == TASK_NUM)
        {
            printf("pi = %.16f\n", pi);
            fflush(stdout);
            break;
        }
        usleep(500000);
    }
    return 0;
}
