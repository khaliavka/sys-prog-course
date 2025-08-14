#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <mqueue.h>
#include <sys/stat.h>
#include <unistd.h>

#define SEND_QUEUE "/server_queue"
#define RCV_QUEUE "/client_queue"
#define MSG_PRIO 0
#define BUF_SZ 100

int main(void)
{
    struct mq_attr attr = {
        .mq_flags = 0,
        .mq_maxmsg = 10,
        .mq_msgsize = BUF_SZ,
        .mq_curmsgs = 0};
    mqd_t smq = mq_open(SEND_QUEUE, O_CREAT | O_RDWR, 0600, &attr);
    if (smq == -1)
    {
        perror("mq_open");
        exit(EXIT_FAILURE);
    }
    // mqd_t rmq = mq_open(RCV_QUEUE, O_CREAT | O_RDWR, 0600, &attr);
    // if (rmq == -1)
    // {
    //     perror("mq_open");
    //     exit(EXIT_FAILURE);
    // }

    for (int i = 0; i < 70; ++i)
    {
        char line[100];
        // printf("Enter a message: ");
        // if (fgets(line, sizeof(line), stdin) == NULL)
        // {
        //     perror("fgets");
        //     exit(EXIT_FAILURE);
        // }
        // line[strcspn(line, "\n")] = '\0';
        snprintf(line, sizeof(line), "sent line: %d", i);
        printf("%s\n", line);
        if (mq_send(smq, line, sizeof(line), MSG_PRIO) == -1)
        {
            perror("mq_send");
            exit(EXIT_FAILURE);
        }
	sleep(1);
    }

    // char msg_rcv[BUF_SZ];
    // ssize_t bytes_read = mq_receive(rmq, msg_rcv, sizeof(msg_rcv), NULL);
    // if (bytes_read == -1)
    // {
    //     perror("mq_receive");
    //     mq_close(rmq);
    //     exit(EXIT_FAILURE);
    // }

    // printf("Server received: %s\n", msg_rcv);

    if (mq_close(smq) == -1)
    {
        perror("mq_close");
    }
    // if (mq_close(rmq) == -1)
    // {
    //     perror("mq_close");
    // }
    if (mq_unlink(SEND_QUEUE) == -1)
    {
        perror("mq_unlink");
    }
    // if (mq_unlink(RCV_QUEUE) == -1)
    // {
    //     perror("mq_unlink");
    // }
    return EXIT_SUCCESS;
}
