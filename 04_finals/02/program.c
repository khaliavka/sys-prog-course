#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>
#include <sys/epoll.h>
#include <sys/wait.h>

#include "exitmacro.h"
#include "commands.h"
#include "drivers_t.h"
#include "console_io.h"
#include "message_t.h"
#include "main_impl.h"

#define MAX_EVENTS (DRIVERS_COUNT + 1)

atomic_int cancel_flag = 0;

void sigint_handler(int sig)
{
    (void)sig;
    cancel_flag = 1;
}

int main(void)
{
    drivers_t drivers;
    drivers_t zombies;
    struct epoll_event events[MAX_EVENTS];

    struct sigaction sa = {.sa_handler = sigint_handler, .sa_flags = 0};
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIGINT, &sa, NULL) == -1)
        err_exit("sigaction");

    int epollfd = epoll_create1(0);
    if (epollfd == -1)
        err_exit("create_epoll");

    epoll_add_fd(epollfd, STDIN_FILENO);

    printf("Usage: command [ARG1] [ARG2]\n"
           "Commands:\n"
           "create_driver\n"
           "delete_driver <pid>\n"
           "send_task <pid> <time>\n"
           "get_status <pid>\n"
           "get_drivers\n");

    while (cancel_flag == 0)
    {
        int nevents = epoll_wait(epollfd, events, MAX_EVENTS, -1);
        if (nevents == -1)
        {
            if (errno == EAGAIN || errno == EINTR)
                continue;
            err_exit("epoll_wait");
        }
        for (int i = 0; i < nevents; ++i)
        {
            if (events[i].data.fd == STDIN_FILENO)
            {
                command_args_t cmd_args;
                if (read_command(&cmd_args) == -1)
                    continue;
                exec_command(&cmd_args, &drivers, &zombies, epollfd);
                continue;
            }
            int zmb_idx = search_driver_by_fd(&zombies, events[i].data.fd);
            if (zmb_idx != -1)
            {
                if (waitpid(zombies.arr[zmb_idx].pid, NULL, 0) == -1)
                    err_exit("waitpid");
                remove_driver(&zombies, zmb_idx);
                epoll_del_fd(epollfd, events[i].data.fd);
                close(events[i].data.fd);
                continue;
            }
            message_t in_msg;
            if (read(events[i].data.fd, &in_msg, sizeof(in_msg)) == -1)
                err_exit("read_af_unix_sock");
            print_message(&in_msg);
        }
    }
    close(epollfd);
    cleanup_drivers(&drivers, close);
    cleanup_drivers(&zombies, close);
    return EXIT_SUCCESS;
}
