#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pthread.h>
#include <stdatomic.h>

#include "connect.h"
#include "exitmacro.h"
#include "client.h"
#include "shmbuf_t.h"

struct shmbuf_t *create_shmqueue(const char *name)
{
    int fd = shm_open(name, O_CREAT | O_EXCL | O_RDWR, 0600);
    if (fd == -1)
        err_exit("shm_open");
    if (ftruncate(fd, sizeof(struct shmbuf_t)) == -1)
        err_exit("ftruncate");
    struct shmbuf_t *shmp = mmap(NULL, sizeof(*shmp), PROT_READ | PROT_WRITE,
                               MAP_SHARED, fd, 0);
    if (shmp == MAP_FAILED)
        err_exit("mmap");
    if (close(fd) == -1)
        err_exit("close");

    shmbuf_init(shmp);

    return shmp;
}

int delete_shmqueue(struct shmbuf_t *shmp, const char *name)
{
    if (munmap(shmp, sizeof(*shmp)) == -1)
        err_exit("munmap");
    if (shm_unlink(name) == -1)
        err_exit("shm_unlink");
    return 0;
}

struct shmbuf_t *open_shmqueue(const char *name)
{
    int fd = shm_open(name, O_RDWR, 0);
    if (fd == -1)
        err_exit("shm_open");
    struct shmbuf_t *shmp = mmap(NULL, sizeof(*shmp), PROT_READ | PROT_WRITE,
                               MAP_SHARED, fd, 0);
    if (shmp == MAP_FAILED)
        err_exit("mmap");
    if (close(fd) == -1)
        err_exit("close");
    return shmp;
}

int close_shmqueue(struct shmbuf_t *shmp, const char *name)
{
    if (munmap(shmp, sizeof(*shmp)) == -1)
        err_exit("munmap");
    return 0;
}