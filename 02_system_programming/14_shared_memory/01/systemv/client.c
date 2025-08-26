#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <unistd.h>

#include "header.h"

union semun
{
    int val;
    struct semid_ds *buf;
    unsigned short *array;
    struct seminfo *__buf;
};

key_t get_key(void)
{
    key_t key = ftok(".", 'a');
    if (key == -1)
    {
        perror("ftok");
        exit(EXIT_FAILURE);
    }
    return key;
}

int sem_wait(int semid, int semnum)
{
    struct sembuf wait_acquire = {semnum, -1, 0};
    while (semop(semid, &wait_acquire, 1) == -1)
    {
        if (errno == EINTR)
            continue;
        perror("semop");
        exit(EXIT_FAILURE);
    }
    return 0;
}

int sem_post(int semid, int semnum)
{
    struct sembuf release = {semnum, 1, 0};
    while (semop(semid, &release, 1) == -1)
    {
        if (errno == EINTR)
            continue;
        perror("semop");
        exit(EXIT_FAILURE);
    }
    return 0;
}

int main(void)
{
    key_t key = get_key();
    int semid = semget(key, NSEMS, 0600);
    if (semid == -1)
    {
        perror("semget");
        exit(EXIT_FAILURE);
    }
    int shmid = shmget(key, BUF_SIZE, 0600);
    if (shmid == -1)
    {
        perror("shmget");
        exit(EXIT_FAILURE);
    }

    
    char *shmem = shmat(shmid, NULL, 0);
    if (shmem == (void *)-1)
    {
        perror("shmat");
        exit(EXIT_FAILURE);
    }

    char buf[BUF_SIZE];
    sem_wait(semid, SEMNUMSERVER);
    strncpy(buf, shmem, sizeof(buf));
    printf("Client received: %s\n", buf);
    
    const char *msg = "Hello!";
    strncpy(shmem, msg, BUF_SIZE);
    sem_post(semid, SEMNUMCLIENT);
    printf("Client sent: %s\n", msg);

    if (shmdt(shmem) == -1)
        err_exit("shmdt");
    return EXIT_SUCCESS;
}
