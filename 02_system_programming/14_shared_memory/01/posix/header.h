#ifndef HEADER_H
#define HEADER_H

#include <semaphore.h>

#define err_exit(msg)    do { perror(msg); exit(EXIT_FAILURE); \
                                   } while (0)

#define SHMEM_OBJ "/shmem_obj"
#define BUF_SIZE 10

struct shmbuf
{
    sem_t sem_srv;
    sem_t sem_clnt;
    size_t cnt;
    char buf[BUF_SIZE];
};

#endif // include guard