#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <mqueue.h>
#include <sys/stat.h>

#define SEND_QUEUE "/client_queue"
#define RCV_QUEUE "/server_queue"
#define MSG_PRIO 0
#define BUF_SZ 256

int main(void)
{
    struct mq_attr attr = {
        .mq_flags = 0,
        .mq_maxmsg = 10,
        .mq_msgsize = BUF_SZ,
        .mq_curmsgs = 0
    };
    mqd_t smq = mq_open(SEND_QUEUE, O_RDWR, 0600, &attr);
    if (smq == -1)
    {
        perror("mq_open");
        exit(EXIT_FAILURE);
    }
    mqd_t rmq = mq_open(RCV_QUEUE, O_RDWR, 0600, &attr);
    if (rmq == -1)
    {
        perror("mq_open");
        exit(EXIT_FAILURE);
    }
    const char *msg_send = "Hello from the client!";
    if (mq_send(smq, msg_send, strlen(msg_send) + 1, MSG_PRIO) == -1)
    {
        perror("mq_send");
        exit(EXIT_FAILURE);
    }
    char msg_rcv[BUF_SZ];
    ssize_t bytes_read = mq_receive(rmq, msg_rcv, sizeof(msg_rcv), NULL);
    if (bytes_read == -1)
    {
        perror("mq_receive");
        mq_close(rmq);
        exit(EXIT_FAILURE);
    }

    printf("Client received: %s\n", msg_rcv);

    if (mq_close(smq) == -1)
    {
        perror("mq_close");
    }
    if (mq_close(rmq) == -1)
    {
        perror("mq_close");
    }
    return EXIT_SUCCESS;
}
