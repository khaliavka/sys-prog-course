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
#include "../commands.h"
#include "../client_settings.h"

#define SLEEPTMSEC 1

atomic_int cancel_flag = 0;

int my_gettime(char *tmbuf, size_t tmbufsz)
{
    time_t current_time = time(NULL);
    struct tm current_time_tm;
    if (strftime(tmbuf, tmbufsz, "%c", localtime_r(&current_time, &current_time_tm)) == 0)
        err_exit("strftime");
    return 0;
}

int get_udp_sock(void)
{
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd == -1)
        err_exit("socket");
    int option = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_BROADCAST, &option, sizeof option) == -1)
        err_exit("setsockopt");
    return fd;
}

void *do_exit_thread(void *args)
{
    char cmd[10];
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
    pthread_t exit_thread;
    int fd;

    pthread_create(&exit_thread, NULL, do_exit_thread, NULL);

    fd = get_udp_sock();
    struct sockaddr_in broadcast_endp;
    memset(&broadcast_endp, 0, sizeof(broadcast_endp));
    broadcast_endp.sin_family = AF_INET;
    broadcast_endp.sin_port = htons(CLIENT_BROADC_PORT);
    if (inet_pton(AF_INET, BROADC_ADDR, &broadcast_endp.sin_addr) < 1)
        err_exit("inet_pton");

    while (cancel_flag == 0)
    {
        char timebuf[TIMEBUFSIZE];
        my_gettime(timebuf, sizeof(timebuf));
        if (sendto(fd, (const char *)timebuf, sizeof(timebuf), 0,
                   (const struct sockaddr *)&broadcast_endp, (socklen_t)sizeof(broadcast_endp)) != sizeof(timebuf))
            err_exit("sendto");
        sleep(SLEEPTMSEC);
    }
    const char command[] = EXITCMD;
    if (sendto(fd, command, sizeof(command), 0,
                   (const struct sockaddr *)&broadcast_endp, (socklen_t)sizeof(broadcast_endp)) != sizeof(command))
            err_exit("sendto");
    
    pthread_join(exit_thread, NULL);
    if (close(fd) == -1)
        err_exit("close");
    return EXIT_SUCCESS;
}
