#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <mqueue.h>
#include <sys/stat.h>
#include <unistd.h>

#define HELLOMQ "/hello_mq"
#define HIMQ "/hi_mq"
#define MSG_PRIO 0
#define BUF_SZ 1024

int main(void)
{
    struct mq_attr attr = {
        .mq_flags = 0,
        .mq_maxmsg = 10,
        .mq_msgsize = BUF_SZ,
        .mq_curmsgs = 0};
    mqd_t hello_mq = mq_open(HELLOMQ, O_CREAT | O_RDWR, 0600, &attr);
    if (hello_mq == -1)
    {
        perror("mq_open");
        exit(EXIT_FAILURE);
    }
    mqd_t hi_mq = mq_open(HIMQ, O_CREAT | O_RDWR, 0600, &attr);
    if (hi_mq == -1)
    {
        perror("mq_open");
        exit(EXIT_FAILURE);
    }

    char hello_buf[BUF_SZ];
    ssize_t bytes_read = mq_receive(hello_mq, hello_buf, sizeof(hello_buf), NULL);
    if (bytes_read == -1)
    {
        perror("mq_receive");
        mq_close(hello_mq);
        exit(EXIT_FAILURE);
    }
    printf("Received hello message: %s\n", hello_buf);

    char *username = strtok(hello_buf, "|");
    char *snd_mqname = strtok(NULL, "|");
    mqd_t send_mq = mq_open(snd_mqname, O_RDWR, 0600, &attr);
    if (send_mq == -1)
    {
        perror("mq_open");
        exit(EXIT_FAILURE);
    }
    const char *test_string = "Hey, solder! Who is in command here? -Motherfucker";
    char bbuff[1024];
    for (int i = 0; i < 60; ++i)
    {

        snprintf(bbuff, sizeof(bbuff), "%s---%d", test_string, i);
        if (mq_send(send_mq, bbuff, sizeof(bbuff), MSG_PRIO) == -1)
        {
            perror("mq_send");
            exit(EXIT_FAILURE);
        }
        sleep(1);
    }
    // for (int i = 0; i < 70; ++i)
    // {
    //     char line[100];
    //     // printf("Enter a message: ");
    //     // if (fgets(line, sizeof(line), stdin) == NULL)
    //     // {
    //     //     perror("fgets");
    //     //     exit(EXIT_FAILURE);
    //     // }
    //     // line[strcspn(line, "\n")] = '\0';
    //     snprintf(line, sizeof(line), "sent line: %d", i);
    //     printf("%s\n", line);
    //     if (mq_send(smq, line, sizeof(line), MSG_PRIO) == -1)
    //     {
    //         perror("mq_send");
    //         exit(EXIT_FAILURE);
    //     }
    // sleep(1);
    // }

    // printf("Server received: %s\n", msg_rcv);

    if (mq_close(hello_mq) == -1)
    {
        perror("mq_close");
    }
    if (mq_close(hi_mq) == -1)
    {
        perror("mq_close");
    }
    if (mq_unlink(HELLOMQ) == -1)
    {
        perror("mq_unlink");
    }
    if (mq_unlink(HIMQ) == -1)
    {
        perror("mq_unlink");
    }
    return EXIT_SUCCESS;
}
