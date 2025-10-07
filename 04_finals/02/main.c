#include <errno.h>
#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>
#include <sys/epoll.h>
#include "exitmacro.h"
#include "commands.h"
#include "drivers_t.h"
#include "input.h"

#define MAX_EVENTS (DRIVERS_COUNT + 1)

atomic_int cancel_flag = 0;

void sigint_handler(int sig)
{
    (void)sig;
    cancel_flag = 1;
}

int main(void)
{
    struct sigaction sa = {.sa_handler = sigint_handler, .sa_flags = 0};
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIGINT, &sa, NULL) == -1)
        err_exit("sigaction");

    drivers_t drivers;
    char inputbuf[INPUTBUF_SIZE];
    command_args_t cmd_args;
    struct epoll_event ev, events[MAX_EVENTS];

    int epollfd = epoll_create1(0);
    if (epollfd == -1)    
        err_exit("create_epoll");

    memset(&ev, 0, sizeof(ev));
    ev.events = EPOLLIN;
    ev.data.fd = STDIN_FILENO;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, STDIN_FILENO, &ev) == -1)
        err_exit("epoll_ctl");

    while (cancel_flag == 0)
    {
        int nevents = epoll_wait(epollfd, events, sizeof(events), -1);
        if (nevents == -1)
        {
            if (errno == EAGAIN || errno == EINTR)
                continue;
            err_exit("epoll_wait");
        }
        // start here (read variant 4 server)
        if (read_line(inputbuf, sizeof(inputbuf)) == -1)
            continue;

        if (parse_command(inputbuf, sizeof(inputbuf), &cmd_args) == -1)
        {
            puts("Bad command.");
            continue;
        }

        switch (cmd_args.cmd)
        {
        case CREATE_DRIVER:
            puts("You entered 'create_driver'");
            if (create_driver(&cmd_args.args.create_driver, &drivers) == -1)
                puts("Cannot create driver.");

            break;
        case SEND_TASK:
            printf("You entered 'send_task %d %ld'\n",
                    cmd_args.args.send_task.driver_pid,
                    cmd_args.args.send_task.busy_secs);
            break;
        case GET_STATUS:
            printf("You entered 'get_status %d'\n",
                    cmd_args.args.get_status.driver_pid);
            break;
        case GET_DRIVERS:
            puts("You entered 'get_drivers'");
            break;
        default:
            break;
        }
    }
    return EXIT_SUCCESS;
}
