#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
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
    if (inet_pton(AF_INET, SRV_IPADDR, &srvaddr.sin_addr) != 1)
        err_exit("inet_pton");
    const char msg[] = "-----------------------------";
    if (sendto(sfd, msg, sizeof(msg), 0, (const struct sockaddr *)&srvaddr, sizeof(srvaddr)) != sizeof(msg))
        err_exit("sendto");
    
    char buf[BUF_SIZE];
    int nr = recvfrom(sfd, buf, sizeof(buf) - 1, 0, NULL, NULL);
    if (nr == -1)
        err_exit("recvfrom");
    buf[nr] = '\0';
    printf("%s\n", buf);

    if (close(sfd) == -1)
        err_exit("close");
    return EXIT_SUCCESS;
}
