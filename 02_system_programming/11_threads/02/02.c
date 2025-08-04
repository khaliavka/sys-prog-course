#include <stdio.h>
#include <pthread.h>

#define NUM_THREADS 16
#define N 10000000

void *thread_calc(void *args)
{
    for (int i = 0; i < N; ++i)
    {
        ++(*(int *)args);
    }
    return NULL;
}

int main(void)
{
    pthread_t thread[NUM_THREADS];
    long array[NUM_THREADS] = {0};
    // map
    for (int i = 0; i < NUM_THREADS; ++i)
    {
        pthread_create(&thread[i], NULL, thread_calc, (void *)&array[i]);
    }
    
    for (int i = 0; i < NUM_THREADS; ++i)
    {
        pthread_join(thread[i], NULL);
    }
    // reduce
    long ans = 0;
    for (int i = 0; i < NUM_THREADS; ++i)
    {
        ans += array[i];
    }
    printf("a = %ld\n", ans);
    return 0;
}