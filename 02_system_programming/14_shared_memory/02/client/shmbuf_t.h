#ifndef SHMBUF_T_H
#define SHMBUF_T_H

#include <semaphore.h>

#define SHMBUF_NAME "/shm_buffer"
#define SHMBUF_MSG_COUNT 4
#define SHMBUF_MSG_SIZE 1024

struct shmbuf_t
{
    sem_t sem;
    unsigned long begin;
    unsigned long end;
    char buf[SHMBUF_MSG_COUNT][SHMBUF_MSG_SIZE];
};

int shmbuf_init(struct shmbuf_t *shmp);
int shmbuf_send(struct shmbuf_t *shmp, const char* msg, int msgsz);
int shmbuf_receive(struct shmbuf_t *shmp, char *msg, int msgsz);

#endif