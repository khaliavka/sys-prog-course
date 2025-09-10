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
#include "fdbuf_t.h"

#define LISTENBACKLOG 5
#define MAX_EVENTS FDBUF_SIZE
#define WAIT_TIMEOUT 500

atomic_int cancel_flag = 0;

int my_gettime(char *tmbuf, size_t tmbufsz)
{
    time_t current_time = time(NULL);
    struct tm current_time_tm;
    if (strftime(tmbuf, tmbufsz, "%c", localtime_r(&current_time, &current_time_tm)) == 0)
        err_exit("strftime");
    return 0;
}

int do_tcp_cleanup(int fd)
{
    char exitcmd[] = EXITCMD;
    if (send(fd, exitcmd, sizeof(exitcmd), 0) != sizeof(exitcmd))
        err_exit("send");
    if (close(fd) == -1)
        err_exit("close");
    return 0;
}

int get_tcp_listen_sock(void)
{
    int listenfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
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
    int sfd = socket(AF_INET, SOCK_DGRAM | SOCK_NONBLOCK, 0);
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
    int nr = recvfrom(fd, (char *)command, sizeof(command) - 1, 0, (struct sockaddr *)claddr, claddrlen);
    if (nr == -1)
        err_exit("recvfrom");
    command[nr] = '\0';
    if (strncmp(command, TIMECMD, sizeof(command)) == 0)
    {
        char timebuf[TIMEBUFSIZE];
        my_gettime(timebuf, sizeof(timebuf));
        if (sendto(fd, (const char *)timebuf, sizeof(timebuf) - 1, 0,
                   (const struct sockaddr *)claddr, sizeof(*claddr)) != sizeof(timebuf) - 1)
            err_exit("sendto");
    }
    return 0;
}

int handle_listen_event(fdbuf_t *fdbuf, int lifd, int epollfd)
{
    int cfd = accept(lifd, NULL, NULL);
    if (cfd == -1)
        err_exit("accept");
    add_fd(fdbuf, cfd, ACTIVE_TCP);

    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = cfd;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, cfd, &ev) == -1)
        err_exit("epoll_ctl");
    return 0;
}

int handle_tcp_event(fdbuf_t *fdbuf, int fd, int epollfd)
{
    char command[CMDBUFSIZE];
    int nr = recv(fd, command, sizeof(command) - 1, 0);
    if (nr == -1)
        err_exit("recv");

    command[nr] = '\0';
    if (strncmp(command, TIMECMD, sizeof(command)) == 0)
    {
        char timebuf[TIMEBUFSIZE];
        my_gettime(timebuf, sizeof(timebuf));
        if (send(fd, timebuf, sizeof(timebuf) - 1, 0) != sizeof(timebuf) - 1)
            err_exit("send");
    }
    else if (strncmp(command, EXITCMD, sizeof(command)) == 0)
    {
        if (epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, NULL) == -1)
            err_exit("epoll_ctl");
        remove_fd(fdbuf, fd, do_tcp_cleanup);
    }
    return 0;
}

void *do_exit_thread(void *args)
{
    char cmd[CMDBUFSIZE];
    while (cancel_flag == 0)
    {
        printf("Toy timeserver (print exit): ");
        fgets(cmd, sizeof(cmd), stdin);
        if (strncmp(cmd, "exit\n", 6) == 0)
            cancel_flag = 1;
    }
    return NULL;
}

int main(void)
{
    fdbuf_t opensocks;
    pthread_t exit_thread;
    struct epoll_event ev, events[MAX_EVENTS];
    int listen_tcpfd, udpfd, epollfd, nevents;
    struct sockaddr_in claddr;
    socklen_t claddrlen;

    pthread_create(&exit_thread, NULL, do_exit_thread, NULL);
    
    memset(&claddr, 0, sizeof(claddr));
    claddrlen = sizeof(claddr);

    udpfd = get_udp_sock();
    listen_tcpfd = get_tcp_listen_sock();

    add_fd(&opensocks, udpfd, UDP);
    add_fd(&opensocks, listen_tcpfd, LISTEN_TCP);

    if ((epollfd = epoll_create1(0)) == -1)
        err_exit("create_epoll");

    memset(&ev, 0, sizeof(ev));
    ev.events = EPOLLIN;
    ev.data.fd = udpfd;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, udpfd, &ev) == -1)
        err_exit("epoll_ctl");

    memset(&ev, 0, sizeof(ev));
    ev.events = EPOLLIN;
    ev.data.fd = listen_tcpfd;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, listen_tcpfd, &ev) == -1)
        err_exit("epoll_ctl");

    while (cancel_flag == 0)
    {
        nevents = epoll_wait(epollfd, events, MAX_EVENTS, WAIT_TIMEOUT);
        if (nevents == -1)
        {
            if (errno == EAGAIN || errno == EINTR)
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

                fddata_t fddata = opensocks.fds[m];
                switch (fddata.type)
                {
                case UDP:
                    handle_udp_event(fddata.fd, &claddr, &claddrlen);
                    break;
                case LISTEN_TCP:
                    handle_listen_event(&opensocks, fddata.fd, epollfd);
                    break;
                case ACTIVE_TCP:
                    handle_tcp_event(&opensocks, fddata.fd, epollfd);
                    break;
                default:
                    break;
                }
            }
        }
    }
    pthread_join(exit_thread, NULL);
    if (close(udpfd) == -1)
        err_exit("close");
    if (close(listen_tcpfd) == -1)
        err_exit("close");
    return EXIT_SUCCESS;
}
