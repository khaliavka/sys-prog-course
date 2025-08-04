#include <stdio.h>
#include <pthread.h>

#define N 5

void *thread_calc(void *args)
{
    printf("thread num = %d\n", *(int *)args);
    return NULL;
}

int main(void)
{
    int *s;
    pthread_t thread[N];
    int index[N];
    for (int i = 0; i < N; ++i)
    {
        index[i] = i + 1;
        pthread_create(&thread[i], NULL, thread_calc, (void *)&index[i]);
    }
    
    for (int i = 0; i < N; ++i)
    {
        pthread_join(thread[i], (void **)&s);
    }
    return 0;
}