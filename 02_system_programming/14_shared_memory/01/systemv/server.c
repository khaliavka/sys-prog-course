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
        err_exit("ftok");
    return key;
}

int sem_init(int semid)
{
    union semun arg;
    arg.val = 0;
    for (int i = 0; i < NSEMS; ++i)
    if (semctl(semid, i, SETVAL, arg) == -1)
        err_exit("semctl");
    return 0;
}

int semsysv_wait(int semid, int semnum)
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

int semsysv_post(int semid, int semnum)
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
    int semid = semget(key, NSEMS, IPC_CREAT | IPC_EXCL | 0600);
    if (semid == -1)
        err_exit("semget");
    sem_init(semid);

    int shmid = shmget(key, BUF_SIZE, IPC_CREAT | 0600);
    if (shmid == -1)
        err_exit("shmget");
    char *shmem = shmat(shmid, NULL, 0);
    if (shmem == (void *)-1)
        err_exit("shmat");

    const char *msg = "Hi!";
    strncpy(shmem, msg, BUF_SIZE);
    semsysv_post(semid, SEMNUMSERVER);
    printf("Server sent: %s\n", msg);
    
    char buf[BUF_SIZE];
    semsysv_wait(semid, SEMNUMCLIENT);
    strncpy(buf, shmem, sizeof(buf));
    printf("Server received: %s\n", buf);

    if (shmdt(shmem) == -1)
        err_exit("shmdt");
    if (shmctl(shmid, IPC_RMID, NULL) == -1)
        err_exit("shmctl");
    if (semctl(semid, 0, IPC_RMID) == -1)
        err_exit("semctl");
    return EXIT_SUCCESS;
}
