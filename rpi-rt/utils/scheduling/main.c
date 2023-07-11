

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
    while (1)
    {
        time_t rawtime;
        struct tm *timeinfo;
        time(&rawtime);
        timeinfo = localtime(&rawtime);
        fflush(stdout);
        fprintf(stderr, "Hello at %s\n", asctime(timeinfo));
        setbuf(stdout, NULL);
        int t = rand() % 10;
        usleep(1000 * (t + 2) * 100);
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
            fflush(stdout);
            fprintf(stderr, "fib(%d)=%d\n",i ,result);
            setbuf(stdout, NULL);
        }
    }
    return NULL;
}

int main(int argc, char *argv[])
{
    // initialize random number generator
    srand(time(NULL));
    struct sched_param param;
    pthread_attr_t norml_attr;
    pthread_t thread;
    int ret;

    /* Lock memory */
    if (mlockall(MCL_CURRENT | MCL_FUTURE) == -1)
    {
        printf("mlockall failed: %m\n");
        exit(-2);
    }

    /* Initialize pthread attributes (default values) */
    ret = pthread_attr_init(&attr);
    if (ret)
    {
        printf("init pthread attributes failed\n");
        goto out;
    }
    /* Set a specific stack size  */
    ret = pthread_attr_setstacksize(&attr, PTHREAD_STACK_MIN);
    if (ret)
    {
        printf("pthread setstacksize failed\n");
        goto out;
    }
    /* Set scheduler policy and priority of pthread */
    ret = pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
    if (ret)
    {
        printf("pthread setschedpolicy failed\n");
        goto out;
    }
    param.sched_priority = 80;
    ret = pthread_attr_setschedparam(&attr, &param);
    if (ret)
    {
        printf("pthread setschedparam failed\n");
        goto out;
    }
    /* Use scheduling parameters of attr */
    ret = pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
    if (ret)
    {
        printf("pthread setinheritsched failed\n");
        goto out;
    }
    /* Create a pthread with specified attributes */
    ret = pthread_create(&thread, &attr, compute_thread_func, NULL);
    if (ret)
    {
        printf("create pthread failed\n");
        goto out;
    }
    /* Join the thread and wait until it is done */
    ret = pthread_join(thread, NULL);
    if (ret)
        printf("join pthread failed: %m\n");

out:
    return ret;
}
