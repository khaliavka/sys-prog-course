#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#include "../exitmacro.h"
#include "../commands.h"
#include "../client_settings.h"

#define REQUESTTIMESEC 1
#define RCVTIMEOUTSEC 5
#define NUMREQUESTS 60

int main(void)
{
    struct sockaddr_in myendp;
    struct timeval tv;
    socklen_t myendplen = sizeof(myendp);
    struct ip_mreq mreq;
    
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd == -1)
        err_exit("socket");

    tv.tv_sec = RCVTIMEOUTSEC;
    tv.tv_usec = 0;
    if (setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv, sizeof(tv)) == -1)
        err_exit("setsockopt SOL_SOCKET");

    memset(&myendp, 0, sizeof(myendp));
    myendp.sin_family = AF_INET;
    myendp.sin_port = htons(CLIENT_PORT);
    myendp.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(fd, (const struct sockaddr *)&myendp, sizeof(myendp)) == -1)
        err_exit("bind");

    if (inet_pton(AF_INET, MULTIC_IPADDR, &mreq.imr_multiaddr.s_addr) != 1)
        err_exit("inet_pton");
    mreq.imr_interface.s_addr = htonl(INADDR_ANY);
    if (setsockopt(fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) == -1)
        err_exit("setsockopt IPPROTO_IP");
    
    for (int i = 0; i < NUMREQUESTS; ++i)
    {
        char timebuf[TIMEBUFSIZE];
        int nr = recvfrom(fd, (char *)timebuf, sizeof(timebuf) - 1, 0,
                          (struct sockaddr *)&myendp, &myendplen);
        if (nr == -1)
        {
            if (errno == EAGAIN)
            {
                puts("Server offline.");
                if (close(fd) == -1)
                    err_exit("close");
                return EXIT_SUCCESS;
            }
            err_exit("recvfrom");
        }
        timebuf[nr] = '\0';
        if (strncmp(timebuf, EXITCMD, sizeof(timebuf)) == 0)
        {
            puts("Server exited.");
            if (close(fd) == -1)
                err_exit("close");
            return EXIT_SUCCESS;
        }
        printf("%s\n", timebuf);
        sleep(REQUESTTIMESEC);
    }
    if (close(fd) == -1)
        err_exit("close");
    return EXIT_SUCCESS;
}
