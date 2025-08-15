#ifndef CONNECT_H
#define CONNECT_H

#include <mqueue.h>

#include "gui.h"

#define MSG_BUFSZ 1024
#define MSGS_COUNT 20
#define USERNM_BUFSZ 21
#define USERNM_COUNT 20
#define MQNAMESZ 256
#define HELLOSZ (MQNAMESZ * 4)

struct msgs_buf_t
{
    char buf[MSGS_COUNT][MSG_BUFSZ];
    int insert_idx;
};

int receive_message(struct msgs_buf_t *msgs_buf, mqd_t rcvmq);

#endif