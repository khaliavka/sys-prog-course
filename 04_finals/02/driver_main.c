#include <errno.h>
#include <stdio.h>
#include <sys/timerfd.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <stdatomic.h>

#include "driver_main.h"
#include "exitmacro.h"
#include "main_impl.h"
#include "message_t.h"

extern atomic_int cancel_flag;

int driver_main(int sockfd)
{
    struct epoll_event events[DRIVER_MAX_EV];

    driver_state_t driver_state = STATE_AVAIL;
    pid_t pid = getpid();
    printf("Driver pid = %d: Created.\n", pid);

    int timerfd = timerfd_create(CLOCK_MONOTONIC, 0);
    if (timerfd == -1)
        err_exit("timerfd_create");

    int epollfd = epoll_create1(0);
    if (epollfd == -1)
        err_exit("create_epoll");

    epoll_add_fd(epollfd, sockfd);
    epoll_add_fd(epollfd, timerfd);

    while (cancel_flag == 0)
    {
        int nevents = epoll_wait(epollfd, events, DRIVER_MAX_EV, -1);
        if (nevents == -1)
        {
            if (errno == EAGAIN || errno == EINTR)
                continue;
            err_exit("epoll_wait");
        }
        for (int i = 0; i < nevents; ++i)
        {
            if (events[i].data.fd == timerfd)
            {
                uint64_t expir_count;
                if (read(timerfd, &expir_count, sizeof(expir_count)) != sizeof(expir_count))
                {
                    if (errno == EAGAIN || errno == EINTR)
                        continue;
                    err_exit("timerfd read");
                }
                driver_state = STATE_AVAIL;
                continue;
            }
            message_t in_msg, out_msg;
            if (read(sockfd, &in_msg, sizeof(in_msg)) == -1)
                err_exit("read_af_unix_sock");
            switch (in_msg.msg_type)
            {
            case MSG_SEND_TASK:
                driver_setbusy(timerfd, in_msg.args.send_task.busy_secs);
                driver_state = STATE_BUSY;
                break;
            case MSG_GET_STATUS:
                switch (driver_state)
                {
                case STATE_AVAIL:
                    out_msg.msg_type = MSG_AVAIL;
                    out_msg.args.avail.pid = pid;
                    break;
                case STATE_BUSY:
                    out_msg.msg_type = MSG_BUSY;
                    out_msg.args.busy.pid = pid;
                    driver_getbusy(timerfd, &out_msg.args.busy.busy_secs);
                    break;
                default:
                    err_exit("driver: unknown driver_state");
                    break;
                }
                if (write(sockfd, &out_msg, sizeof(out_msg)) == -1)
                    err_exit("write_af_unix_sock");
                break;
            case MSG_DEL:
                cancel_flag = 1;
                break;
            default:
                err_exit("driver: unknown msg_type");
                break;
            }
        }
    }
    close(timerfd);
    close(epollfd);
    close(sockfd);
    printf("Driver pid = %d: Exited gracefully\n", pid);
    return 0;
}