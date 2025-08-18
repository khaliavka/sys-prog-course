#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <mqueue.h>
#include <sys/random.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <unistd.h>
#include <pthread.h>

#include "client.h"
#include "gui.h"
#include "connect.h"

#define HELLOMQ "/hello_mq"

int main(void)
{
    struct windows_t windows;
    char my_username[USERNM_BUFSZ];
    char rcv_mqname[MQNAMESZ];
    char snd_mqname[MQNAMESZ];
    char rcv_uname_mqname[MQNAMESZ];
    char hello_buf[HELLOSZ];
    unsigned long mqid;

    init_gui();
    draw_gui(&windows);
    get_box_input((MSGWND_ROWS + INPUTWND_ROWS + 2) / 2,
                  (INPUTWND_COLS - USERNAME_COLS - 19) / 2,
                  my_username, USERNM_BUFSZ, "Enter your username");
    refresh_windows(&windows);

    struct mq_attr attr = {
        .mq_flags = 0,
        .mq_maxmsg = 10,
        .mq_msgsize = 1024,
        .mq_curmsgs = 0};

    getrandom(&mqid, sizeof(mqid), 0);

    snprintf(rcv_mqname, sizeof(rcv_mqname), "/%s_msg_in_%lu", my_username, mqid);
    mqd_t rcv_msg_mq = mq_open(rcv_mqname, O_CREAT | O_EXCL | O_RDWR | O_NONBLOCK, 0600, &attr);
    if (rcv_msg_mq == -1)
    {
        perror("mq_open");
        exit(EXIT_FAILURE);
    }
    snprintf(snd_mqname, sizeof(snd_mqname), "/%s_msg_out_%lu", my_username, mqid);
    mqd_t snd_msg_mq = mq_open(snd_mqname, O_CREAT | O_EXCL | O_RDWR | O_NONBLOCK, 0600, &attr);
    if (snd_msg_mq == -1)
    {
        perror("mq_open");
        exit(EXIT_FAILURE);
    }
    snprintf(rcv_uname_mqname, sizeof(rcv_uname_mqname), "/%s_username_in_%lu", my_username, mqid);
    mqd_t rcv_uname_mq = mq_open(rcv_uname_mqname, O_CREAT | O_EXCL | O_RDWR | O_NONBLOCK, 0600, &attr);
    if (rcv_uname_mq == -1)
    {
        perror("mq_open");
        exit(EXIT_FAILURE);
    }
    mqd_t hello_mq = mq_open(HELLOMQ, O_WRONLY, 0600, &attr);
    if (hello_mq == -1)
    {
        perror("mq_open");
        exit(EXIT_FAILURE);
    }

    snprintf(hello_buf, sizeof(hello_buf), "%s|%s|%s|%s", my_username, rcv_mqname, snd_mqname, rcv_uname_mqname);
    if (mq_send(hello_mq, hello_buf, sizeof(hello_buf), 0) == -1)
    {
        perror("mq_send");
        exit(EXIT_FAILURE);
    }

    pthread_t msgs_thread;
    pthread_t usrnms_thread;
    pthread_t inputmsg_thread;

    struct args_t msgs_args = {.wnd = windows.messages.wnd, .mqd = rcv_msg_mq};
    pthread_create(&msgs_thread, NULL, do_msgs_thread, &msgs_args);

    struct args_t usrnms_args = {.wnd = windows.users.wnd, .mqd = rcv_uname_mq};
    pthread_create(&usrnms_thread, NULL, do_usernames_thread, &usrnms_args);

    struct args_t inputmsg_args = {.wnd = windows.input.wnd, .mqd = snd_msg_mq};
    pthread_create(&inputmsg_thread, NULL, do_input_msg_thread, &inputmsg_args);

    pthread_join(msgs_thread, NULL);
    pthread_join(usrnms_thread, NULL);
    pthread_join(inputmsg_thread, NULL);

    struct mq_attr hello_attr;
    if (mq_getattr(hello_mq, &hello_attr) == -1)
    {
        perror("mq_getattr");
        exit(EXIT_FAILURE);
    }
    hello_attr.mq_flags |= O_NONBLOCK;
    if (mq_setattr(hello_mq, &hello_attr, NULL) == -1)
    {
        perror("mq_setattr");
        exit(EXIT_FAILURE);
    }
    const char *exit_msg = "exit_msg";
    if (mq_send(hello_mq, exit_msg, strlen(exit_msg) + 1, 0) == -1)
    {
        perror("mq_send");
        exit(EXIT_FAILURE);
    }

    if (mq_close(rcv_msg_mq) == -1)
    {
        perror("mq_close");
    }
    if (mq_close(snd_msg_mq) == -1)
    {
        perror("mq_close");
    }
    if (mq_close(rcv_uname_mq) == -1)
    {
        perror("mq_close");
    }
    if (mq_close(hello_mq) == -1)
    {
        perror("mq_close");
    }
    if (mq_unlink(rcv_mqname) == -1)
    {
        perror("mq_unlink");
    }
    if (mq_unlink(snd_mqname) == -1)
    {
        perror("mq_unlink");
    }
    if (mq_unlink(rcv_uname_mqname) == -1)
    {
        perror("mq_unlink");
    }
    cleanup_gui(&windows);
    return EXIT_SUCCESS;
}
