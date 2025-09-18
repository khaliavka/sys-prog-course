#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/random.h>
#include <unistd.h>

#include "../exitmacro.h"
#include "../settings.h"

int main(void)
{
    int sfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sfd == -1)
        err_exit("socket");

    struct sockaddr_in srvaddr;
    memset(&srvaddr, 0, sizeof(srvaddr));
    srvaddr.sin_family = AF_INET;
    srvaddr.sin_port = htons(SRV_PORT);
    if (inet_pton(AF_INET, SRV_IPADDR, &srvaddr.sin_addr) < 1)
        err_exit("inet_pton");
    if (bind(sfd, (const struct sockaddr *)&srvaddr, sizeof(srvaddr)) == -1)
        err_exit("bind");

    struct sockaddr_in claddr;
    socklen_t len = sizeof(claddr);
    memset(&claddr, 0, sizeof(claddr));
    char buf[BUF_SIZE];
    while (1)
    {
        int nr = recvfrom(sfd, buf, sizeof(buf) - 1, 0, (struct sockaddr *)&claddr, &len);
        if (nr == -1)
            err_exit("recvfrom");
        size_t index;
        if (getrandom(&index, sizeof(index), 0) == -1)
            err_exit("getrandom");
        buf[index % (nr - 1)] = '*';
        buf[nr] = '\0';
        printf("%s\n", buf);
        if (sendto(sfd, buf, nr, 0, (const struct sockaddr *)&claddr, sizeof(claddr)) != nr)
            err_exit("sendto");
    }

    if (close(sfd) == -1)
        err_exit("close");
    return EXIT_SUCCESS;
}
