#include "../exitmacro.h"
#include "taskqueue_t.h"

static int taskqueue_is_empty(struct taskqueue_t *tqp)
{
    return tqp->begin == tqp->end;
}

static int taskqueue_is_full(struct taskqueue_t *tqp)
{
    return (tqp->begin % TASKBUF_SIZE) == (tqp->end % TASKBUF_SIZE) &&
           !taskqueue_is_empty(tqp);
}

int taskqueue_init(struct taskqueue_t *tqp)
{
    pthread_mutex_init(&tqp->mtx, NULL);
    pthread_cond_init(&tqp->condvar, NULL);
    tqp->begin = 0;
    tqp->end = 0;
    return 0;
}

int taskqueue_push(struct taskqueue_t *tqp, struct task_t tsk)
{
    pthread_mutex_lock(&tqp->mtx);
    if (taskqueue_is_full(tqp) || tqp->cancel_flag == 1)
    {
        pthread_mutex_unlock(&tqp->mtx);
        return -1;
    }
    tqp->tskbuf[tqp->end++] = tsk;
    pthread_cond_signal(&tqp->condvar);
    pthread_mutex_unlock(&tqp->mtx);
    return 0;
}

int taskqueue_pop(struct taskqueue_t *tqp, struct task_t *tskp)
{
    pthread_mutex_lock(&tqp->mtx);
    while (taskqueue_is_empty(tqp) && tqp->cancel_flag == 0)
        pthread_cond_wait(&tqp->condvar, &tqp->mtx);
    if (tqp->cancel_flag == 1)
    {
        pthread_mutex_unlock(&tqp->mtx);
        return -1;
    }
    *tskp = tqp->tskbuf[tqp->begin++];
    pthread_mutex_unlock(&tqp->mtx);
    return 0;
}

int taskqueue_cancel(struct taskqueue_t *tqp, int (*do_cleanup)(struct task_t))
{
    tqp->cancel_flag = 1;
    pthread_cond_broadcast(&tqp->condvar);
    pthread_mutex_lock(&tqp->mtx);
    for (unsigned long i = tqp->begin; i != tqp->end; ++i)
    {
        do_cleanup(tqp->tskbuf[i % TASKBUF_SIZE]);
    }
    pthread_mutex_unlock(&tqp->mtx);
    return 0;
}