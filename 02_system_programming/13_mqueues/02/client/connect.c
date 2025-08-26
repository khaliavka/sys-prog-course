#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <mqueue.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pthread.h>
#include <stdatomic.h>

#include "connect.h"
#include "client.h"

#define USLEEPTM 100000
#define SHORT_PROMPT "Message"

extern pthread_mutex_t ncurses_mtx;
_Atomic int cancel_flag = 0;

struct msgs_buf_t
{
    char buf[MSGS_COUNT][MSG_BUFSZ];
    int insert_idx;
};

int get_insert_idx(struct msgs_buf_t *buf)
{
    return buf->insert_idx;
}

char (*get_buf(struct msgs_buf_t *buf))[MSG_BUFSZ]
{
    return buf->buf;
}

int receive_message(mqd_t mqd, struct msgs_buf_t *msgs_buf)
{
    int idx = msgs_buf->insert_idx % MSGS_COUNT;

    if (mq_receive(mqd, (char *)&msgs_buf->buf[idx], sizeof(msgs_buf->buf[0]), NULL) != -1)
    {
        ++(msgs_buf->insert_idx);
        return 0;
    }
    if (errno == EAGAIN)
        return -1;

    perror("mq_receive");
    mq_close(mqd);
    exit(EXIT_FAILURE);
    return -1;
}

int receive_usernames(mqd_t mqd, char *usernames, int sz)
{
    if (mq_receive(mqd, usernames, sz, NULL) != -1)
        return 0;
    if (errno == EAGAIN)
        return -1;

    perror("mq_receive");
    mq_close(mqd);
    exit(EXIT_FAILURE);
    return -1;
}

int send_input(mqd_t mqd, char *input_msg, int sz)
{
    if (mq_send(mqd, input_msg, sz, 0) != -1)
        return 0;
    if (errno == EAGAIN)
        return -1;

    perror("mq_send");
    mq_close(mqd);
    exit(EXIT_FAILURE);
    return -1;
}

void *do_msgs_thread(void *args)
{
    struct msgs_buf_t messages = {.insert_idx = 0};

    WINDOW *wnd = ((struct args_t *)args)->wnd;
    mqd_t mqd = ((struct args_t *)args)->mqd;
    while (atomic_load(&cancel_flag) == 0)
    {
        if (receive_message(mqd, &messages) == -1)
        {
            usleep(USLEEPTM);
            continue;
        }
        pthread_mutex_lock(&ncurses_mtx);
        print_msgs_buf(wnd, &messages);
        pthread_mutex_unlock(&ncurses_mtx);
    }
    return NULL;
}

void *do_usernames_thread(void *args)
{
    char usernames[1024];
    WINDOW *wnd = ((struct args_t *)args)->wnd;
    mqd_t mqd = ((struct args_t *)args)->mqd;
    while (atomic_load(&cancel_flag) == 0)
    {
        if (receive_usernames(mqd, usernames, sizeof(usernames)) == -1)
        {
            usleep(USLEEPTM);
            continue;
        }
        pthread_mutex_lock(&ncurses_mtx);
        print_buffer(wnd, usernames);
        pthread_mutex_unlock(&ncurses_mtx);
    }
    return NULL;
}

void *do_input_msg_thread(void *args)
{
    char input_msg[MSG_BUFSZ];
    WINDOW *wnd = ((struct args_t *)args)->wnd;
    mqd_t mqd = ((struct args_t *)args)->mqd;
    while (atomic_load(&cancel_flag) == 0)
    {
        read_input(wnd, input_msg, sizeof(input_msg), SHORT_PROMPT);
        if (strncmp(input_msg, "exit", 5) == 0)
        {
            atomic_store(&cancel_flag, 1);
        }
        if (send_input(mqd, input_msg, sizeof(input_msg)) == -1)
            usleep(USLEEPTM);
    }
    return NULL;
}
