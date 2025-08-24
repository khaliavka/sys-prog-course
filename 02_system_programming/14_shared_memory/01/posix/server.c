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
    int shmfd = shm_open(SHMEM_OBJ, O_CREAT | O_EXCL | O_RDWR, 0600);
    if (shmfd == -1)
        err_exit("shm_open");
    if (ftruncate(shmfd, sizeof(struct shmbuf)) == -1)
        err_exit("ftruncate");
    struct shmbuf *shmp = mmap(NULL, sizeof(*shmp), PROT_READ | PROT_WRITE,
        MAP_SHARED, shmfd, 0);
    if (shmp == MAP_FAILED)
        err_exit("mmap");
    if (sem_init(&shmp->sem_srv, 1, 0) == -1)
        err_exit("sem_init-sem_srv");
    if (sem_init(&shmp->sem_clnt, 1, 0) == -1)
        err_exit("sem_init-sem_clnt");

    const char *msg = "Hi!";
    strncpy(shmp->buf, msg, sizeof(shmp->buf));
    if (sem_post(&shmp->sem_srv) == -1)
        err_exit("sem_post-sem_srv");
    printf("Server sent: %s\n", msg);
    
    char buf[BUF_SIZE];
    if (sem_wait(&shmp->sem_clnt) == -1)
        err_exit("sem_wait-sem_clnt");
    strncpy(buf, shmp->buf, sizeof(shmp->buf));
    printf("Server received: %s\n", buf);

    if (close(shmfd) == -1)
        err_exit("close");
    if (shm_unlink(SHMEM_OBJ) == -1)
        err_exit("shm_unlink");
    return EXIT_SUCCESS;
}
