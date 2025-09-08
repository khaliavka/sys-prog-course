#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#include "../exitmacro.h"
#include "../srvsettings.h"
#include "../commands.h"

#define NUMREQUESTS 30
#define REQUESTTIMESEC 1

int main(void)
{
    int sfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sfd == -1)
        err_exit("socket");

    struct sockaddr_in srvaddr;
    socklen_t srvaddrlen = sizeof(srvaddr);
    memset(&srvaddr, 0, sizeof(srvaddr));
    srvaddr.sin_family = AF_INET;
    srvaddr.sin_port = htons(SRVPORT);
    if (inet_pton(AF_INET, SRVADDR, &srvaddr.sin_addr) < 1)
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
            err_exit("recvfrom");
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
