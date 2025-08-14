#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <mqueue.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <unistd.h>

#include "client.h"
#include "gui.h"
#include "connect.h"

#define RCV_QUEUE "/server_queue"

int print_msgs_buf(WINDOW *wnd, struct ringbuf *messages)
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

int main(void)
{
    struct ringbuf messages = {.insert_idx = 0};
    struct windows windows;
    init_gui();
    draw_gui(&windows);
    char nickname[NICKNAME_BUFSZ];

    // get_box_input((MSGWND_ROWS + INPUTWND_ROWS + 2) / 2, (INPUTWND_COLS - NICKNAME_COLS - 19) / 2, nickname, NICKNAME_BUFSZ, "Enter your nickname");
    // refresh_windows(&windows);
    // char input_msg[MSG_BUFSZ];
    // read_input(windows.input.wnd, input_msg, sizeof(input_msg), "Message");

    struct mq_attr attr = {
        .mq_flags = 0,
        .mq_maxmsg = 100,
        .mq_msgsize = MSG_BUFSZ,
        .mq_curmsgs = 0};

    mqd_t rmq = mq_open(RCV_QUEUE, O_RDWR, 0600, &attr);
    if (rmq == -1)
    {
        perror("mq_open");
        exit(EXIT_FAILURE);
    }

    while(1)
    {
        receive_message(&messages, rmq);
        print_msgs_buf(windows.messages.wnd, &messages);
    }
    // print_buffer(windows.users.wnd, msg_buf);

    getch();
    cleanup_gui(&windows);
    return EXIT_SUCCESS;
}
