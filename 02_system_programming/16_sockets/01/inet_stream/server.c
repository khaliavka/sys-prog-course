#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#include "exitmacro.h"
#include "srvaddrport.h"

#define BACKLOG 1
#define BUF_SIZE 100

int main(void)
{
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd == -1)
        err_exit("socket");
    int option = 1;
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option)) == -1)
        err_exit("setsockopt");
    
    struct sockaddr_in srvaddr;
    memset(&srvaddr, 0, sizeof(srvaddr));
    srvaddr.sin_family = AF_INET;
    srvaddr.sin_port = htons(SRVPORT);
    if (inet_pton(AF_INET, SRVADDR, &srvaddr.sin_addr) < 1)
        err_exit("inet_pton");
    if (bind(listenfd, (const struct sockaddr *)&srvaddr, sizeof(srvaddr)) == -1)
        err_exit("bind");

    if (listen(listenfd, BACKLOG) == -1)
        err_exit("listen");
    
    char buf[BUF_SIZE];
    int connfd = accept(listenfd, NULL, NULL);
    if (connfd == -1)
        err_exit("accept");
    
    int nr = recv(connfd, buf, sizeof(buf) - 1, 0);
    if (nr == -1)
        err_exit("recv");
    buf[nr] = '\0';
    printf("Client sent: %s\n", buf);

    const char msg[] = "hi!";
    if (send(connfd, msg, sizeof(msg), 0) != sizeof(msg))
        err_exit("sendto");
    
    if (close(connfd) == -1)
        err_exit("close");
    if (close(listenfd) == -1)
        err_exit("close");
    return EXIT_SUCCESS;
}
