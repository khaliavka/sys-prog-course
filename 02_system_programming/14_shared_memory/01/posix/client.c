#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <unistd.h>

#include "header.h"

int main(void)
{
    int shmfd = shm_open(SHMEM_OBJ, O_RDWR, 0);
    if (shmfd == -1)
        err_exit("shm_open");
    
    struct shmbuf *shmp = mmap(NULL, sizeof(*shmp), PROT_READ | PROT_WRITE,
        MAP_SHARED, shmfd, 0);
    if (shmp == MAP_FAILED)
        err_exit("mmap");

    char buf[BUF_SIZE];
    sem_wait(&shmp->sem_srv);
    strncpy(buf, shmp->buf, sizeof(shmp->buf));
    printf("Client received: %s\n", buf);

    const char *msg = "Hello!";
    strncpy(shmp->buf, msg, sizeof(shmp->buf));
    if (sem_post(&shmp->sem_clnt) == -1)
        err_exit("sem_post-sem_srv");
    printf("Client sent: %s\n", msg);

    if (close(shmfd) == -1)
        err_exit("close");
    return EXIT_SUCCESS;
}
