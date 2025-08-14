#ifndef CONNECT_H
#define CONNECT_H

#include <mqueue.h>

#include "gui.h"

#define MSG_BUFSZ 100
#define NICKNAME_BUFSZ 20
#define MSGS_COUNT 20

struct ringbuf
{
    char buf[MSGS_COUNT][MSG_BUFSZ];
    int insert_idx;
};

int receive_message(struct ringbuf *msgs_buf, mqd_t rcvmq);

#endif