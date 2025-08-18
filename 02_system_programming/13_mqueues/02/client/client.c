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

mqd_t open_mqueue(char *mqname, int sz, const char* pattern, char * username, unsigned long mqid, struct mq_attr *attr)
{
    snprintf(mqname, sz, pattern, username, mqid);
    mqd_t mqd = mq_open(mqname, O_CREAT | O_EXCL | O_RDWR | O_NONBLOCK, 0600, attr);
    if (mqd == -1)
    {
        perror("mq_open");
        exit(EXIT_FAILURE);
    }
    return mqd;
}

int delete_mqueue(mqd_t mqd, char *mqname)
{
    if (mq_close(mqd) == -1)
    {
        perror("mq_close");
        exit(EXIT_FAILURE);
    }
    if (mq_unlink(mqname) == -1)
    {
        perror("mq_unlink");
        exit(EXIT_FAILURE);
    }
    return 0;
}

int main(void)
{
    struct windows_t windows;
    const char *patterns[3] = {"/%s_msg_in_%lu", "/%s_username_in_%lu", "/%s_msg_out_%lu", };
    char my_username[USERNM_BUFSZ];
    char mqnames[3][MQNAMESZ];
    mqd_t mqds[3];

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
    
    for (int i = 0; i < 3; ++i)
    {
        mqds[i] = open_mqueue((char *)&mqnames[i], sizeof(mqnames[0]), patterns[i], my_username, mqid, &attr);
    }
    mqd_t hello_mq = mq_open(HELLOMQ, O_WRONLY, 0600, &attr);
    if (hello_mq == -1)
    {
        perror("mq_open");
        exit(EXIT_FAILURE);
    }
    snprintf(hello_buf, sizeof(hello_buf), "%s|%s|%s|%s", my_username, mqnames[0], mqnames[1], mqnames[2]);
    if (mq_send(hello_mq, hello_buf, sizeof(hello_buf), 0) == -1)
    {
        perror("mq_send");
        exit(EXIT_FAILURE);
    }
    pthread_t msgs_thread;
    pthread_t usrnms_thread;
    pthread_t inputmsg_thread;

    struct args_t msgs_args = {.wnd = windows.messages.wnd, .mqd = mqds[0]};
    pthread_create(&msgs_thread, NULL, do_msgs_thread, &msgs_args);
    struct args_t usrnms_args = {.wnd = windows.users.wnd, .mqd = mqds[1]};
    pthread_create(&usrnms_thread, NULL, do_usernames_thread, &usrnms_args);
    struct args_t inputmsg_args = {.wnd = windows.input.wnd, .mqd = mqds[2]};
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
    if (mq_close(hello_mq) == -1)
    {
        perror("mq_close");
    }
    for (int i = 0; i < 3; ++i)
    {
        delete_mqueue(mqds[i], (char *)&mqnames[i]);
    }
    cleanup_gui(&windows);
    return EXIT_SUCCESS;
}
