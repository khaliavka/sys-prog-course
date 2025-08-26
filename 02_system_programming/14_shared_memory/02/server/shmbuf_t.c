#include <stdio.h>
#include <string.h>
#include <semaphore.h>

#include "exitmacro.h"
#include "shmbuf_t.h"

int shmbuf_init(struct shmbuf_t *shmp)
{
    if (sem_init(&shmp->sem, 1, 1) == -1)
        err_exit("sem_init");
    shmp->begin = 0;
    shmp->end = 0;
    return 0;
}

static int shmbuf_is_empty(struct shmbuf_t *shmp)
{
    if (shmp->begin == shmp->end)
        return 1;
    return 0;    
}

static int shmbuf_is_full(struct shmbuf_t *shmp)
{
    if ((shmp->begin % SHMBUF_MSG_COUNT) == (shmp->end % SHMBUF_MSG_COUNT) &&
        !shmbuf_is_empty(shmp))
        return 1;
    return 0;
}

int shmbuf_send(struct shmbuf_t *shmp, const char* msg, int msgsz)
{
    sem_wait(&shmp->sem);
    if (shmbuf_is_full(shmp))
    {
        sem_post(&shmp->sem);
        return -1;
    }
    int sz = (msgsz < SHMBUF_MSG_SIZE) ? msgsz : SHMBUF_MSG_SIZE;
    strncpy((char *)&shmp->buf[shmp->end % SHMBUF_MSG_COUNT], msg, sz);
    ++(shmp->end);
    sem_post(&shmp->sem);
    return 0;
}

int shmbuf_receive(struct shmbuf_t *shmp, char *msg, int msgsz)
{
    sem_wait(&shmp->sem);
    if (shmbuf_is_empty(shmp))
    {
        sem_post(&shmp->sem);
        return -1;
    }
    int sz = (msgsz < SHMBUF_MSG_SIZE) ? msgsz : SHMBUF_MSG_SIZE;
    strncpy(msg, shmp->buf[shmp->begin % SHMBUF_MSG_COUNT], sz);
    ++(shmp->begin);
    sem_post(&shmp->sem);
    return 0;
}
