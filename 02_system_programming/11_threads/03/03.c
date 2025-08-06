#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <sys/random.h>
#include <unistd.h>

#define ARRAY_SIZE 5
#define NUM_CONSUMERS 3
#define NUM_PRODUCERS 1
#define MAX_DEMAND 100000
#define SUPPLY_BATCH 5000
#define CONSUMER_SLEEP_TIME 2
#define PRODUCER_SLEEP_TIME 1

int stores[ARRAY_SIZE];
pthread_mutex_t mutexes[ARRAY_SIZE];

void init_rand(struct drand48_data *rd)
{
    long seed;
    getrandom(&seed, sizeof(seed), 0);
    srand48_r(seed, rd);
}

long get_rand_idx(struct drand48_data *rd)
{
    long index;
    lrand48_r(rd, &index);
    return index % ARRAY_SIZE;
}

void *consumer(void *args)
{
    struct drand48_data rd;
    init_rand(&rd);
    int demand = MAX_DEMAND;

    while (demand)
    {
        long index = get_rand_idx(&rd);
        int prev_demand = demand;

        pthread_mutex_lock(&mutexes[index]);
        int stock = stores[index];
        int amount = ((stock < demand) ? stock : demand);
        stores[index] -= amount;
        pthread_mutex_unlock(&mutexes[index]);

        demand -= amount;

        printf("consumer %d: previous demand = %d, index = %ld, stock = %d, new demand = %d\n"
            , *(int *)args, prev_demand, index, stock, demand);
        sleep(CONSUMER_SLEEP_TIME);
    }
    return NULL;
}

void *producer(void *args)
{

    struct drand48_data rd;
    init_rand(&rd);

    // this thread must be cancelled
    // by main
    while (1)
    {
        long index = get_rand_idx(&rd);

        pthread_mutex_lock(&mutexes[index]);
        int stock = stores[index];
        stores[index] += SUPPLY_BATCH;
        pthread_mutex_unlock(&mutexes[index]);

        printf("producer %d: index = %ld, stock = %d + %d\n", *(int *)args, index, stock, SUPPLY_BATCH);
        sleep(PRODUCER_SLEEP_TIME);
    }
    return NULL;
}

void init(void)
{
    struct drand48_data rd;
    init_rand(&rd);

    for (int i = 0; i < ARRAY_SIZE; ++i)
    {
        long n;
        lrand48_r(&rd, &n);
        stores[i] = n % SUPPLY_BATCH + SUPPLY_BATCH;
        pthread_mutex_init(&mutexes[i], NULL);
    }
}

int main(void)
{

    init();

    pthread_t consumers[NUM_CONSUMERS];
    int consumer_index[NUM_CONSUMERS];

    for (int i = 0; i < NUM_CONSUMERS; ++i)
    {
        consumer_index[i] = i + 1;
        pthread_create(&consumers[i], NULL, consumer, &consumer_index[i]);
    }

    pthread_t producers[NUM_PRODUCERS];
    int producer_index[NUM_PRODUCERS];

    for (int i = 0; i < NUM_PRODUCERS; ++i)
    {
        producer_index[i] = i + 1;
        pthread_create(&producers[i], NULL, producer, &producer_index[i]);
    }

    for (int i = 0; i < NUM_CONSUMERS; ++i)
    {
        pthread_join(consumers[i], NULL);
    }

    for (int i = 0; i < NUM_PRODUCERS; ++i)
    {
        pthread_cancel(producers[i]);
    }

    for (int i = 0; i < NUM_PRODUCERS; ++i)
    {
        pthread_join(producers[i], NULL);
    }
    return 0;
}