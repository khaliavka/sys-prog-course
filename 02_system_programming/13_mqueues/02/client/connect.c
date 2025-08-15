#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <mqueue.h>
#include <sys/stat.h>

#include "connect.h"

int receive_message(struct msgs_buf_t *msgs_buf, mqd_t rcvmq)
{
    int idx = msgs_buf->insert_idx % MSGS_COUNT;
    ssize_t bytes_read = mq_receive(rcvmq, (char *)&msgs_buf->buf[idx], sizeof(msgs_buf->buf[0]), NULL);
    if (bytes_read == -1)
    {
        perror("mq_receive");
        mq_close(rcvmq);
        exit(EXIT_FAILURE);
    }
    ++(msgs_buf->insert_idx);
    return 0;
}
