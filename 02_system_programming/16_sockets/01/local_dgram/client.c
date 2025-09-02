#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include "exitmacro.h"
#include "srvsoname.h"

#define BUF_SIZE 100
#define CLSONAME "/tmp/client_hi_sock"
int main(void)
{
    int sfd = socket(AF_LOCAL, SOCK_DGRAM, 0);
    if (    sfd == -1)
        err_exit("socket");

    struct sockaddr_un claddr;
    memset(&claddr, 0, sizeof(claddr));
    claddr.sun_family = AF_LOCAL;
    snprintf(claddr.sun_path, sizeof(claddr.sun_path), CLSONAME"_%ld", (long) getpid());
    if (bind(sfd, (const struct sockaddr *)&claddr, sizeof(claddr)) == -1)
        err_exit("bind");
    
    struct sockaddr_un srvaddr;
    memset(&srvaddr, 0, sizeof(srvaddr));
    srvaddr.sun_family = AF_LOCAL;
    strncpy(srvaddr.sun_path, SRVSONAME, sizeof(claddr.sun_path) - 1);

    const char msg[] = "hello!";
    if (sendto(sfd, msg, sizeof(msg), 0, (struct sockaddr *)&srvaddr, sizeof(srvaddr)) != sizeof(msg))
        err_exit("sendto");
    
    char buf[BUF_SIZE];
    int nr = recvfrom(sfd, buf, sizeof(buf) - 1, 0, NULL, NULL);
    if (nr == -1)
        err_exit("recvfrom");
    buf[nr] = '\0';
    printf("Server sent: %s\n", buf);

    if (close(sfd) == -1)
        err_exit("close");
    if (remove(claddr.sun_path) == -1)
        err_exit("remove");
    return EXIT_SUCCESS;
}
