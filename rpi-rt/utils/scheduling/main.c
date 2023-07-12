

/*
 * POSIX Real Time Example
 * using a single pthread as RT thread
 */

#include <limits.h>
#include <pthread.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>

#define FIB_LENGTH 30

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
    req.tv_nsec = 0; // 2000ms
    req.tv_sec = 2;
    while (1)
    {
        time_t rawtime;
        struct tm *timeinfo;
        time(&rawtime);
        timeinfo = localtime(&rawtime);
        printf("Hello at %s\n", asctime(timeinfo));
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
            printf("fib(%d)=%d\n", i, result);
        }
    }
    return NULL;
}


    int
    main(int argc, char *argv[])
{
    // initialize random number generator
    srand(time(NULL));
    struct sched_param critical_param, param;
    pthread_attr_t compute_attr, io_attr;
    pthread_t compute_thread, io_thread;
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
    ret = pthread_attr_setschedpolicy(&compute_attr, SCHED_FIFO);
    if (ret)
    {
        printf("pthread setschedpolicy failed\n");
        goto out;
    }
    ret = pthread_attr_setschedpolicy(&io_attr, SCHED_FIFO);
    if (ret)
    {
        printf("pthread setschedpolicy failed\n");
        goto out;
    }
    critical_param.sched_priority = sched_get_priority_max(SCHED_FIFO);
    param.sched_priority = sched_get_priority_min(SCHED_FIFO);
    ret = pthread_attr_setschedparam(&compute_attr, &critical_param);
    if (ret)
    {
        printf("pthread setschedparam failed\n");
        goto out;
    }
    ret = pthread_attr_setschedparam(&io_attr, &param);
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
    ret = pthread_create(&compute_thread, &compute_attr, compute_thread_func, NULL);
    if (ret)
    {
        printf("error code:%d\n", ret);
        printf("create compute pthread failed\n");
        goto out;
    }
    ret = pthread_create(&io_thread, &io_attr, io_thread_func, NULL);
    if (ret)
    {
        printf("error code:%d\n", ret);
        printf("create io pthread failed\n");
        goto out;
    }
    /* Join the thread and wait until it is done */
    ret = pthread_join(compute_thread, NULL);
    if (ret)
        printf("join pthread failed: %m\n");
    ret = pthread_join(io_thread, NULL);
    if (ret)
        printf("join pthread failed: %m\n");

out:
    return ret;
}
