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
    int sfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sfd == -1)
        err_exit("socket");

    struct sockaddr_in srvaddr;
    memset(&srvaddr, 0, sizeof(srvaddr));
    srvaddr.sin_family = AF_INET;
    srvaddr.sin_port = htons(SRVPORT);
    if (inet_pton(AF_INET, SRVADDR, &srvaddr.sin_addr) < 1)
        err_exit("inet_pton");
    if (connect(sfd, (const struct sockaddr *)&srvaddr, sizeof(srvaddr)))
        err_exit("connect");
    
    const char msg[] = "hello!";
    if (send(sfd, msg, sizeof(msg), 0) != sizeof(msg))
        err_exit("send");
    
    char buf[BUF_SIZE];
    int nr = recv(sfd, buf, sizeof(buf) - 1, 0);
    if (nr == -1)
        err_exit("recv");
    buf[nr] = '\0';
    printf("Server sent: %s\n", buf);

    if (close(sfd) == -1)
        err_exit("close");
    return EXIT_SUCCESS;
}
