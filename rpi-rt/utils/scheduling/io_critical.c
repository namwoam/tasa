

/*
 * POSIX Real Time Example
 * using a single pthread as RT thread
 */

#define _GNU_SOURCE

#include <limits.h>
#include <pthread.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>

#define FIB_LENGTH 30
#define SCHEDULE_POLICY SCHED_FIFO
#define COMPUTE_THREAD_COUNT 10

int fib(int n)
{
    if (n == 0 || n == 1)
    {
        return 1;
    }
    else
    {
        return fib(n - 1) + fib(n - 2);
    }
}

void *io_thread_func(void *data)
{
    struct timespec req, rem;
    req.tv_nsec = 500000000; // 0.5sec
    req.tv_sec = 0;
    while (1)
    {
        time_t rawtime;
        struct tm *timeinfo;
        time(&rawtime);
        timeinfo = localtime(&rawtime);
        printf("Hello at %s, on thread:%d\n", asctime(timeinfo), sched_getcpu());
        int t = rand() % 10;
        if (nanosleep(&req, &rem) == -1)
        {
            printf("error at nanosleep!\n");
            exit(1);
        }
    }
    return NULL;
}

void *compute_thread_func(void *data)
{
    int result;
    while (1)
    {
        for (int i = 0; i < 20; i++)
        {
            result = fib(i);
        }
    }
    return NULL;
}

int main(int argc, char *argv[])
{
    // initialize random number generator
    srand(time(NULL));
    struct sched_param critical_param, param;
    pthread_attr_t compute_attr, io_attr;
    pthread_t compute_threads[COMPUTE_THREAD_COUNT];
    pthread_t io_thread;
    int ret;

    /* Lock memory */
    if (mlockall(MCL_CURRENT | MCL_FUTURE) == -1)
    {
        printf("mlockall failed: %m\n");
        exit(-2);
    }

    /* Initialize pthread attributes (default values) */
    ret = pthread_attr_init(&compute_attr);
    if (ret)
    {
        printf("init pthread attributes failed\n");
        goto out;
    }
    ret = pthread_attr_init(&io_attr);
    if (ret)
    {
        printf("init pthread attributes failed\n");
        goto out;
    }
    /* Set a specific stack size  */
    ret = pthread_attr_setstacksize(&compute_attr, PTHREAD_STACK_MIN);
    if (ret)
    {
        printf("pthread setstacksize failed\n");
        goto out;
    }
    ret = pthread_attr_setstacksize(&io_attr, PTHREAD_STACK_MIN);
    if (ret)
    {
        printf("pthread setstacksize failed\n");
        goto out;
    }
    /* Set scheduler policy and priority of pthread */
    ret = pthread_attr_setschedpolicy(&compute_attr, SCHEDULE_POLICY);
    if (ret)
    {
        printf("pthread setschedpolicy failed\n");
        goto out;
    }
    ret = pthread_attr_setschedpolicy(&io_attr, SCHEDULE_POLICY);
    if (ret)
    {
        printf("pthread setschedpolicy failed\n");
        goto out;
    }
    printf("max priproity:%d\n", sched_get_priority_max(SCHEDULE_POLICY));
    printf("min priproity:%d\n", sched_get_priority_min(SCHEDULE_POLICY));
    // ref:https://man7.org/linux/man-pages/man7/sched.7.html
    critical_param.sched_priority = sched_get_priority_max(SCHEDULE_POLICY);
    param.sched_priority = sched_get_priority_min(SCHEDULE_POLICY);
    ret = pthread_attr_setschedparam(&compute_attr, &param);
    if (ret)
    {
        printf("pthread setschedparam failed\n");
        goto out;
    }
    ret = pthread_attr_setschedparam(&io_attr, &critical_param);
    if (ret)
    {
        printf("pthread setschedparam failed\n");
        goto out;
    }
    /* Use scheduling parameters of attr */
    ret = pthread_attr_setinheritsched(&compute_attr, PTHREAD_EXPLICIT_SCHED);
    if (ret)
    {
        printf("pthread setinheritsched failed\n");
        goto out;
    }
    ret = pthread_attr_setinheritsched(&io_attr, PTHREAD_EXPLICIT_SCHED);
    if (ret)
    {
        printf("pthread setinheritsched failed\n");
        goto out;
    }
    /* Create a pthread with specified attributes */
    printf("Start execution:\n");
    for (int i = 0; i < COMPUTE_THREAD_COUNT; i++)
    {
        ret = pthread_create(&compute_threads[i], &compute_attr, compute_thread_func, NULL);
        if (ret)
        {
            printf("error code:%d\n", ret);
            printf("create compute pthread failed\n");
            goto out;
        }
    }

    ret = pthread_create(&io_thread, &io_attr, io_thread_func, NULL);
    if (ret)
    {
        printf("error code:%d\n", ret);
        printf("create io pthread failed\n");
        goto out;
    }
    /* Join the thread and wait until it is done */
    for (int i = 0; i < COMPUTE_THREAD_COUNT; i++)
    {
        ret = pthread_join(compute_threads[i], NULL);
        if (ret)
            printf("join pthread failed: %m\n");
    }
    ret = pthread_join(io_thread, NULL);
    if (ret)
        printf("join pthread failed: %m\n");

out:
    return ret;
}
