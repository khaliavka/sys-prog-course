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
#include "taskqueue_t.h"

#define LISTENBACKLOG 5
#define RCVTIMEOUTSEC 1
#define NUMWORKERS 16

struct taskqueue_t tasks;
pthread_t worker_tids[NUMWORKERS];

int my_gettime(char *tmbuf, size_t tmbufsz)
{
    time_t current_time = time(NULL);
    struct tm current_time_tm;
    if (strftime(tmbuf, tmbufsz, "%c", localtime_r(&current_time, &current_time_tm)) == 0)
        err_exit("strftime");
    return 0;
}

int send_exitcmd(struct task_t task)
{
    char exitcmd[] = EXITCMD;
    if (send(task.connfd, exitcmd, sizeof(exitcmd), 0) != sizeof(exitcmd))
        err_exit("send");
    if (close(task.connfd) == -1)
        err_exit("close");
    return 0;
}

void *do_connection_thread(void *args)
{
    struct task_t task = {.connfd = -1};
    while (tasks.cancel_flag == 0)
    {
        if (taskqueue_pop(&tasks, &task) == -1)
            break;
        while (tasks.cancel_flag == 0)
        {
            char command[CMDBUFSIZE];
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
    return NULL;
}

void *do_exit_thread(void *args)
{
    char cmd[CMDBUFSIZE];
    while (tasks.cancel_flag == 0)
    {
        printf("Toy timeserver (print exit): ");
        fgets(cmd, sizeof(cmd), stdin);
        if (strncmp(cmd, "exit\n", 6) == 0)
            taskqueue_cancel(&tasks, send_exitcmd);
    }
    return NULL;
}

int get_listen_sock(void)
{
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd == -1)
        err_exit("socket");
    int option = 1;
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option)) == -1)
        err_exit("setsockopt");
    struct timeval tv;
    tv.tv_sec = RCVTIMEOUTSEC;
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
    if (listen(listenfd, LISTENBACKLOG) == -1)
        err_exit("listen");
    return listenfd;
}

int main(void)
{
    int listenfd = get_listen_sock();
    taskqueue_init(&tasks);

    for (int i = 0; i < NUMWORKERS; ++i)
    {
        pthread_create(&worker_tids[i], NULL, do_connection_thread, NULL);
    }
    pthread_t exit_thr;
    pthread_create(&exit_thr, NULL, do_exit_thread, NULL);

    while (tasks.cancel_flag == 0)
    {
        int connfd = accept(listenfd, NULL, NULL);
        if (connfd == -1)
        {
            if (errno = EAGAIN)
                continue;
            err_exit("accept");
        }
        struct timeval tv;
        tv.tv_sec = RCVTIMEOUTSEC;
        tv.tv_usec = 0;
        if (setsockopt(connfd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv, sizeof tv) == -1)
            err_exit("setsockopt");
        if (taskqueue_push(&tasks, (struct task_t) {connfd}) == -1)
            if (close(connfd) == -1)
                err_exit("close");
    }
    for (int i = 0; i < NUMWORKERS; ++i)
        pthread_join(worker_tids[i], NULL);
    pthread_join(exit_thr, NULL);
    if (close(listenfd) == -1)
        err_exit("close");
    return EXIT_SUCCESS;
}
