#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#include "exitmacro.h"
#include "srvaddrport.h"

#define BUF_SIZE 100

int main(void)
{
    int sfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sfd == -1)
        err_exit("socket");

    struct sockaddr_in srvaddr;
    memset(&srvaddr, 0, sizeof(srvaddr));
    srvaddr.sin_family = AF_INET;
    srvaddr.sin_addr.s_addr = inet_addr(SRVADDR);
    srvaddr.sin_port = htons(SRVPORT);
    if (bind(sfd, (const struct sockaddr *)&srvaddr, sizeof(srvaddr)) == -1)
        err_exit("bind");

    char buf[BUF_SIZE];
    struct sockaddr_in claddr;
    socklen_t len = sizeof(claddr);
    memset(&claddr, 0, sizeof(claddr));
    int nr = recvfrom(sfd, buf, sizeof(buf) - 1, 0, (struct sockaddr *)&claddr, &len);
    if (nr == -1)
        err_exit("recvfrom");
    buf[nr] = '\0';
    printf("Client sent: %s\n", buf);

    const char msg[] = "hi!";
    if (sendto(sfd, msg, sizeof(msg), 0, (const struct sockaddr *)&claddr, sizeof(claddr)) != sizeof(msg))
        err_exit("sendto");
    
    if (close(sfd) == -1)
        err_exit("close");
    return EXIT_SUCCESS;
}
