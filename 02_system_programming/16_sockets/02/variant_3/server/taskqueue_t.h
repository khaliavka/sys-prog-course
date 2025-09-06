#ifndef TASKQUEUE_T_H
#define TASKQUEUE_T_H

#include <pthread.h>
#include <stdatomic.h>

#define TASKBUF_SIZE 32

struct task_t
{
    int connfd;
};

struct taskqueue_t
{
    pthread_mutex_t mtx;
    pthread_cond_t condvar;
    atomic_int cancel_flag;
    unsigned long begin;
    unsigned long end;
    struct task_t tskbuf[TASKBUF_SIZE];
};

int taskqueue_init(struct taskqueue_t *tqp);
int taskqueue_push(struct taskqueue_t *tqp, struct task_t tsk);
int taskqueue_pop(struct taskqueue_t *tqp, struct task_t *tskp);
int taskqueue_cancel(struct taskqueue_t *tqp, int (*do_cleanup)(struct task_t));

#endif