#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#include "../exitmacro.h"
#include "../settings.h"
#include "../commands.h"

#define NUMREQUESTS 30
#define REQUESTTIMESEC 1
#define RCVTIMEOUTSEC 5

int main(void)
{
    int sfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sfd == -1)
        err_exit("socket");

    struct timeval tv;
    tv.tv_sec = RCVTIMEOUTSEC;
    tv.tv_usec = 0;
    if (setsockopt(sfd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv, sizeof tv) == -1)
        err_exit("setsockopt");

    struct sockaddr_in srvaddr;
    socklen_t srvaddrlen = sizeof(srvaddr);
    memset(&srvaddr, 0, sizeof(srvaddr));
    srvaddr.sin_family = AF_INET;
    srvaddr.sin_port = htons(SRV_PORT);
    if (inet_pton(AF_INET, SRV_IPADDR, &srvaddr.sin_addr) < 1)
        err_exit("inet_pton");
    
    const char timecmd[] = TIMECMD;
    for (int i = 0; i < NUMREQUESTS; ++i)
    {
        if (sendto(sfd, timecmd, sizeof(timecmd), 0,
            (const struct sockaddr *)&srvaddr, sizeof(srvaddr)) != sizeof(timecmd))
                err_exit("sendto");
        char timebuf[TIMEBUFSIZE];
        int nr = recvfrom(sfd, (char *)timebuf, sizeof(timebuf) - 1, 0, (struct sockaddr *)&srvaddr, &srvaddrlen);
        if (nr == -1)
        {
            if (errno == EAGAIN)
            {
                puts("Server offline.");
                if (close(sfd) == -1)
                err_exit("close");
                return EXIT_SUCCESS;
            }
            err_exit("recvfrom");
        }
        timebuf[nr] = '\0';
        if (strncmp(timebuf, EXITCMD, sizeof(timebuf)) == 0)
        {
            puts("Server exited.");
            if (close(sfd) == -1)
                err_exit("close");
            return EXIT_SUCCESS;
        }
        printf("%s\n", timebuf);
        sleep(REQUESTTIMESEC);
    }
    if (close(sfd) == -1)
        err_exit("close");
    return EXIT_SUCCESS;
}
