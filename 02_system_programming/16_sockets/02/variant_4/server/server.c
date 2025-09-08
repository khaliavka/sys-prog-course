#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <stdatomic.h>

#include "../exitmacro.h"
#include "../srvsettings.h"
#include "../commands.h"

#define LISTENBACKLOG 5
#define FDBUF_SIZE 1000
#define MAX_EVENTS FDBUF_SIZE
#define WAIT_TIMEOUT 500

typedef enum
{
    UDP,
    LISTEN_TCP,
    ACTIVE_TCP
} fdtype_t;

typedef struct
{
    int fd;
    fdtype_t type;
} fddata_t;

typedef struct
{
    fddata_t fds[FDBUF_SIZE];
    int count;
} fdbuf_t;

atomic_int cancel_flag = 0;
fdbuf_t opensocks;

int add_fd(fdbuf_t *fdbuf, int fd, fdtype_t fdtype)
{
    if (fdbuf->count == FDBUF_SIZE)
        return -1;
    fdbuf->fds[fdbuf->count++] = (fddata_t){.fd = fd, .type = fdtype};
    return 0;
}

int remove_fd(fdbuf_t *fdbuf, int fd, int (*do_cleanup)(int))
{
    int index = 0;
    for (int i = 0; i < fdbuf->count; ++i)
    {
        if (fdbuf->fds[i].fd == fd)
        {
            fdbuf->fds[i] = fdbuf->fds[fdbuf->count - 1];
            --fdbuf->count;
            do_cleanup(fd);
            break;
        }
    }
    return 0;
}

int my_gettime(char *tmbuf, size_t tmbufsz)
{
    time_t current_time = time(NULL);
    struct tm current_time_tm;
    if (strftime(tmbuf, tmbufsz, "%c", localtime_r(&current_time, &current_time_tm)) == 0)
        err_exit("strftime");
    return 0;
}

int send_exittcp(int fd)
{
    char exitcmd[] = EXITCMD;
    if (send(fd, exitcmd, sizeof(exitcmd), 0) != sizeof(exitcmd))
        err_exit("send");
    if (close(fd) == -1)
        err_exit("close");
    return 0;
}

void *do_connection_thread(void *args)
{
    /*
    struct task_t task = {.connfd = -1};
    while (tasks.cancel_flag == 0)
    {
        if (taskqueue_pop(&tasks, &task) == -1)
            break;
        while (tasks.cancel_flag == 0)
        {
            int nr = recv(task.connfd, command, sizeof(command) - 1, 0);
            if (nr == -1)
            {
                if (errno == EAGAIN)
                continue;

                err_exit("recv");
                }
                command[nr] = '\0';
                if (strncmp(command, EXITCMD, sizeof(command)) == 0)
                {
                    if (close(task.connfd) == -1)
                    err_exit("close");
                    task.connfd = -1;
                    break;
                    }
                    if (strncmp(command, TIMECMD, sizeof(command)) == 0)
                    {
                        char timebuf[TIMEBUFSIZE];
                        my_gettime(timebuf, sizeof(timebuf));
                        if (send(task.connfd, timebuf, sizeof(timebuf) - 1, 0) != sizeof(timebuf) - 1)
                        err_exit("send");
                        }
                        }
                        }
                        if (task.connfd == -1)
                        return NULL;
                        send_exitcmd(task);
                        */
    return NULL;
}

void *do_exit_thread(void *args)
{
    /*
char cmd[CMDBUFSIZE];
while (tasks.cancel_flag == 0)
{
printf("Toy timeserver (print exit): ");
fgets(cmd, sizeof(cmd), stdin);
if (strncmp(cmd, "exit\n", 6) == 0)
taskqueue_cancel(&tasks, send_exitcmd);
}
*/
    return NULL;
}

int get_tcp_listen_sock(void)
{
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd == -1)
        err_exit("socket");
    int option = 1;
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option)) == -1)
        err_exit("setsockopt");
    struct sockaddr_in srvaddr;
    memset(&srvaddr, 0, sizeof(srvaddr));
    srvaddr.sin_family = AF_INET;
    srvaddr.sin_port = htons(SRVPORT);
    if (inet_pton(AF_INET, SRVADDR, &srvaddr.sin_addr) < 1)
        err_exit("inet_pton");
    if (bind(listenfd, (const struct sockaddr *)&srvaddr, sizeof(srvaddr)) == -1)
        err_exit("bind");
    if (listen(listenfd, LISTENBACKLOG) == -1)
        err_exit("listen");
    return listenfd;
}

int get_udp_sock(void)
{
    int sfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sfd == -1)
        err_exit("socket");

    struct sockaddr_in srvaddr;
    memset(&srvaddr, 0, sizeof(srvaddr));
    srvaddr.sin_family = AF_INET;
    srvaddr.sin_port = htons(SRVPORT);
    if (inet_pton(AF_INET, SRVADDR, &srvaddr.sin_addr) < 1)
        err_exit("inet_pton");
    if (bind(sfd, (const struct sockaddr *)&srvaddr, sizeof(srvaddr)) == -1)
        err_exit("bind");
    return sfd;
}

int handle_udp_event(int fd, struct sockaddr_in *claddr, socklen_t *claddrlen)
{
    char command[CMDBUFSIZE];
    int nr = recvfrom(fd, (char *)command, sizeof(command) - 1, 0, (struct sockaddr *)&claddr, &claddrlen);
    if (nr == -1)
        err_exit("recvfrom");
    command[nr] = '\0';
    if (strncmp(command, TIMECMD, sizeof(command)) == 0)
    {
        char timebuf[TIMEBUFSIZE];
        my_gettime(timebuf, sizeof(timebuf));
        if (sendto(fd, (const char *)timebuf, sizeof(timebuf) - 1, 0,
                   (const struct sockaddr *)&claddr, sizeof(claddr)) != sizeof(timebuf) - 1)
            err_exit("sendto");
    }
}

int handle_listen_event()
{

}

int main(void)
{
    // pthread_t exit_thr;
    // pthread_create(&exit_thr, NULL, do_exit_thread, NULL);
    struct epoll_event ev, events[MAX_EVENTS];
    int listen_tcpfd, udpfd, epollfd, nevents;
    struct sockaddr_in claddr;
    socklen_t claddrlen;

    memset(&claddr, 0, sizeof(claddr));
    claddrlen = sizeof(claddr);
    udpfd = get_udp_sock();
    listen_tcpfd = get_tcp_listen_sock();
    add_fd(&opensocks, udpfd, UDP);
    add_fd(&opensocks, listen_tcpfd, LISTEN_TCP);

    if ((epollfd = epoll_create1(0)) == -1)
        err_exit("create_epoll");
    ev.events = EPOLLIN;
    ev.data.fd = udpfd;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, udpfd, &ev) == -1)
        err_exit("epoll_ctl");

    while (cancel_flag == 0)
    {
        nevents = epoll_wait(epollfd, events, MAX_EVENTS, WAIT_TIMEOUT);
        if (nevents == -1)
        {
            if (errno == EAGAIN)
                continue;
            err_exit("epoll_wait");
        }

        // O^2 (todo: write hashmap)
        for (int n = 0; n < nevents; ++n)
        {
            for (int m = 0; m < opensocks.count; ++m)
            {
                if (events[n].data.fd != opensocks.fds[m].fd)
                    continue;
                fddata_t fddat = opensocks.fds[m];
                switch (fddat.type)
                {
                case UDP:
                    handle_udp_event(fddat.fd, &claddr, &claddrlen);
                    break;
                case LISTEN_TCP:

                    break;
                case ACTIVE_TCP:
                default:
                    break;
                }
            }
        }
    }
    while (cancel_flag == 0)
    {

        // int connfd = accept(listenfd, NULL, NULL);
        // if (connfd == -1)
        // {
        //     if (errno = EAGAIN)
        //         continue;
        //     err_exit("accept");
        // }
        // struct timeval tv;
        // tv.tv_sec = RCVTIMEOUTSEC;
        // tv.tv_usec = 0;
        // if (setsockopt(connfd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv, sizeof tv) == -1)
        //     err_exit("setsockopt");
        // if (close(connfd) == -1)
        //         err_exit("close");
    }
    /* pthread_join(exit_thr, NULL);
    if (close(listenfd) == -1)
        err_exit("close");
    */
    return EXIT_SUCCESS;
}
