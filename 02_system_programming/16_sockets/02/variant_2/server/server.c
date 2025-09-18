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
#include "../settings.h"
#include "../commands.h"

#define BACKLOG 300
#define RCVTIMEOUTSEC 1
#define NUMWORKERS 16

struct task_t
{
    int index;
    int is_busy;
    int connfd;
    pthread_t worker_tid;
    pthread_mutex_t mutex;
    pthread_cond_t condvar;
};

atomic_int shutdown_flag = 0;
struct task_t tasks[NUMWORKERS];

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
    int index = *(int *)args;
    int connfd = -1;
    while (shutdown_flag == 0)
    {
        pthread_mutex_lock(&tasks[index].mutex);
        tasks[index].is_busy = 0;
        tasks[index].connfd = -1;
        while (tasks[index].is_busy == 0 && shutdown_flag == 0)
            pthread_cond_wait(&tasks[index].condvar, &tasks[index].mutex);
        connfd = tasks[index].connfd;
        pthread_mutex_unlock(&tasks[index].mutex);

        while (shutdown_flag == 0)
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
                break;
            }
            if (strncmp(command, TIMECMD, sizeof(command)) == 0)
            {
                char timebuf[TIMEBUFSIZE];
                my_gettime(timebuf, sizeof(timebuf));
                if (send(connfd, timebuf, sizeof(timebuf) - 1, 0) != sizeof(timebuf) - 1)
                    err_exit("send");
            }
        }
    }
    if (connfd == -1)
        return NULL;
    
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
    while (shutdown_flag == 0)
    {
        printf("Toy timeserver (print exit): ");
        fgets(cmd, sizeof(cmd), stdin);
        if (strncmp(cmd, "exit\n", 6) == 0)
        {
            shutdown_flag = 1;
            for (int i = 0; i < NUMWORKERS; ++i)
            {
                pthread_cond_signal(&tasks[i].condvar);
            }
        }
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
    tv.tv_sec = RCVTIMEOUTSEC;
    tv.tv_usec = 0;
    if (setsockopt(listenfd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv, sizeof tv) == -1)
        err_exit("setsockopt");
    struct sockaddr_in srvaddr;
    memset(&srvaddr, 0, sizeof(srvaddr));
    srvaddr.sin_family = AF_INET;
    srvaddr.sin_port = htons(SRV_PORT);
    if (inet_pton(AF_INET, SRV_IPADDR, &srvaddr.sin_addr) < 1)
        err_exit("inet_pton");
    if (bind(listenfd, (const struct sockaddr *)&srvaddr, sizeof(srvaddr)) == -1)
        err_exit("bind");
    if (listen(listenfd, BACKLOG) == -1)
        err_exit("listen");

    for (int i = 0; i < NUMWORKERS; ++i)
    {
        tasks[i].index = i;
        tasks[i].connfd = -1;
        tasks[i].is_busy = 0;
        pthread_mutex_init(&tasks[i].mutex, NULL);
        pthread_cond_init(&tasks[i].condvar, NULL);
        pthread_create(&tasks[i].worker_tid, NULL, do_connection_thread, (void *)&tasks[i].index);
    }

    pthread_t exit_thr;
    pthread_create(&exit_thr, NULL, do_exit_thread, NULL);

    while (shutdown_flag == 0)
    {
        int connfd = accept(listenfd, NULL, NULL);
        if (connfd == -1)
        {
            if (errno == EAGAIN || errno == EINTR)
                continue;
            err_exit("accept");
        }
        struct timeval tv;
        tv.tv_sec = RCVTIMEOUTSEC;
        tv.tv_usec = 0;
        if (setsockopt(connfd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv, sizeof tv) == -1)
            err_exit("setsockopt");
        int idx;
        for (idx = 0; idx < NUMWORKERS; ++idx)
        {
            pthread_mutex_lock(&tasks[idx].mutex);
            if (tasks[idx].is_busy == 0)
            {
                tasks[idx].is_busy = 1;
                tasks[idx].connfd = connfd;
                pthread_cond_signal(&tasks[idx].condvar);
                pthread_mutex_unlock(&tasks[idx].mutex);
                break;
            }
            pthread_mutex_unlock(&tasks[idx].mutex);
        }
        if (idx == NUMWORKERS)
            if (close(connfd) == -1)
                err_exit("close");
    }

    for (int i = 0; i < NUMWORKERS; ++i)
    {
        pthread_join(tasks[i].worker_tid, NULL);
    }
    pthread_join(exit_thr, NULL);
    if (close(listenfd) == -1)
        err_exit("close");
    return EXIT_SUCCESS;
}
