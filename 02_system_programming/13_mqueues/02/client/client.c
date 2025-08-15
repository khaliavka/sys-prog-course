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
#define HIMQ "/hi_mq"

    struct msgs_args_t {
        WINDOW *wnd;
        mqd_t mqd;
    }; 

int print_msgs_buf(WINDOW *wnd, struct msgs_buf_t *messages)
{
    int end = messages->insert_idx;
    int begin = (end < MSGS_COUNT) ? 0 : end - MSGS_COUNT;
    werase(wnd);
    for (int i = begin; i < end; ++i)
    {
        int idx = i % MSGS_COUNT;
        print_line(wnd, i - begin, 0, (char *)&messages->buf[idx]);
    }
    return 0;
}

int print_names_buf(WINDOW *wnd, char (*name)[USERNM_BUFSZ], int rows)
{
    werase(wnd);
    for (int i = 0; i < rows; ++i)
    {
        print_line(wnd, i, 0, (char *)&name[i]);
    }
    return 0;
}

void *do_msgs_thread(void *args)
{
    struct msgs_buf_t messages = {.insert_idx = 0};
    WINDOW *wnd = ((struct msgs_args_t *)args)->wnd;
    mqd_t mqd = ((struct msgs_args_t *)args)->mqd;
    while(1)
    {
        receive_message(&messages, mqd);
        print_msgs_buf(wnd, &messages);
    }
    return NULL;
}

void *unames_thrd(void *args)
{

    return NULL;
}

void *input_msg_thrd(void *args)
{
    return NULL;
}

int main(void)
{
    struct windows_t windows;
    char usernames[USERNM_COUNT][USERNM_BUFSZ];
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
    mqd_t rcv_msg_mq = mq_open(rcv_mqname, O_CREAT | O_EXCL | O_RDWR, 0600, &attr);
    if (rcv_msg_mq == -1)
    {
        perror("mq_open");
        exit(EXIT_FAILURE);
    }
    snprintf(snd_mqname, sizeof(snd_mqname), "/%s_msg_out_%lu", my_username, mqid);
    mqd_t snd_msg_mq = mq_open(snd_mqname, O_CREAT | O_EXCL | O_RDWR, 0600, &attr);
    if (snd_msg_mq == -1)
    {
        perror("mq_open");
        exit(EXIT_FAILURE);
    }
    snprintf(rcv_uname_mqname, sizeof(rcv_uname_mqname), "/%s_username_in_%lu", my_username, mqid);
    mqd_t rcv_uname_mq = mq_open(rcv_uname_mqname, O_CREAT | O_EXCL | O_RDWR, 0600, &attr);
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

    struct msgs_args_t msgs_args = {.wnd = windows.messages.wnd, .mqd = rcv_msg_mq};
    pthread_create(&msgs_thread, NULL, do_msgs_thread, &msgs_args);
    pthread_join(msgs_thread, NULL);
    // for (int i = 0; i < NICKNM_COUNT; ++i)
    // {
    //     snprintf((char *)&names[i], sizeof(names[0]), "%s", "Arenius");
    // }
    // print_names_buf(windows.users.wnd, names, NICKNM_COUNT);
    // refresh_windows(&windows);
    // char input_msg[MSG_BUFSZ];
    // read_input(windows.input.wnd, input_msg, sizeof(input_msg), "Message");

    // struct mq_attr attr = {
    //     .mq_flags = 0,
    //     .mq_maxmsg = 10,
    //     .mq_msgsize = MSG_BUFSZ,
    //     .mq_curmsgs = 0};

    // mqd_t rmq = mq_open(RCV_QUEUE, O_RDWR, 0600, &attr);
    // if (rmq == -1)
    // {
    //     perror("mq_open");
    //     exit(EXIT_FAILURE);
    // }

    // while(1)
    // {
    //     receive_message(&messages, rmq);
    //     print_msgs_buf(windows.messages.wnd, &messages);
    // }
    // print_buffer(windows.users.wnd, msg_buf);

    getch();
    cleanup_gui(&windows);
    return EXIT_SUCCESS;
}
