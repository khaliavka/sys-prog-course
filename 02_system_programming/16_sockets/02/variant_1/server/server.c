#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <stdatomic.h>

#include "../exitmacro.h"
#include "../srvsettings.h"
#include "../commands.h"

#define BACKLOG 5
#define RCVTIMEOUT 1

atomic_int cancel_flag = 0;

int my_gettime(char *tmbuf, size_t tmbufsz)
{
    time_t current_time = time(NULL);
    struct tm current_time_tm;
    if (strftime(tmbuf, tmbufsz, "%c", localtime_r(&current_time, &current_time_tm)) == 0)
        err_exit("strftime");
    return 0;
}

void *do_connection_thread(void *args)
{
    int connfd = *(int *)args;
    free(args);

    while (cancel_flag == 0)
    {

        char command[CMDBUFSIZE];
        int nr = recv(connfd, command, sizeof(command) - 1, 0);
        if (nr == -1)
        {
            if (errno == EAGAIN)
                continue;

            err_exit("recv");
        }
        command[nr] = '\0';
        if (strncmp(command, EXITCMD, sizeof(command)) == 0)
        {
            if (close(connfd) == -1)
                err_exit("close");
            return NULL;
        }
        if (strncmp(command, TIMECMD, sizeof(command)) == 0)
        {
            char timebuf[TIMEBUFSIZE];
            my_gettime(timebuf, sizeof(timebuf));
            if (send(connfd, timebuf, sizeof(timebuf) - 1, 0) != sizeof(timebuf) - 1)
                err_exit("send");
        }
    }
    char exitcmd[] = EXITCMD;
    if (send(connfd, exitcmd, sizeof(exitcmd), 0) != sizeof(exitcmd))
        err_exit("send");
    if (close(connfd) == -1)
        err_exit("close");
    return NULL;
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
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd == -1)
        err_exit("socket");
    int option = 1;
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option)) == -1)
        err_exit("setsockopt");
    struct timeval tv;
    tv.tv_sec = RCVTIMEOUT;
    tv.tv_usec = 0;
    if (setsockopt(listenfd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv, sizeof tv) == -1)
        err_exit("setsockopt");
    struct sockaddr_in srvaddr;
    memset(&srvaddr, 0, sizeof(srvaddr));
    srvaddr.sin_family = AF_INET;
    srvaddr.sin_port = htons(SRVPORT);
    if (inet_pton(AF_INET, SRVADDR, &srvaddr.sin_addr) < 1)
        err_exit("inet_pton");
    if (bind(listenfd, (const struct sockaddr *)&srvaddr, sizeof(srvaddr)) == -1)
        err_exit("bind");
    if (listen(listenfd, BACKLOG) == -1)
        err_exit("listen");

    pthread_t exit_thr;
    pthread_create(&exit_thr, NULL, do_exit_thread, NULL);

    while (cancel_flag == 0)
    {
        int connfd = accept(listenfd, NULL, NULL);
        if (connfd == -1)
        {
            if (errno == EAGAIN || errno == EINTR)
                continue;
            err_exit("accept");
        }
        struct timeval tv;
        tv.tv_sec = RCVTIMEOUT;
        tv.tv_usec = 0;
        if (setsockopt(connfd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv, sizeof tv) == -1)
            err_exit("setsockopt");
        int *connfdp = malloc(sizeof(*connfdp));
        if (connfdp == NULL)
            err_exit("malloc");
        *connfdp = connfd;
        pthread_t thr;
        pthread_create(&thr, NULL, do_connection_thread, (void *)connfdp);
        pthread_detach(thr);
    }

    pthread_join(exit_thr, NULL);
    if (close(listenfd) == -1)
        err_exit("close");
    return EXIT_SUCCESS;
}
