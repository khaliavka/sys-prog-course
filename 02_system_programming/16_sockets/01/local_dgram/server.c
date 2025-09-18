#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include "exitmacro.h"
#include "settings.h"

int main(void)
{
    int sfd = socket(AF_LOCAL, SOCK_DGRAM, 0);
    if (sfd == -1)
        err_exit("socket");

    struct sockaddr_un srvaddr;
    memset(&srvaddr, 0, sizeof(srvaddr));
    srvaddr.sun_family = AF_LOCAL;
    strncpy(srvaddr.sun_path, SRV_SONAME, sizeof(srvaddr.sun_path) - 1);
    if (bind(sfd, (const struct sockaddr *)&srvaddr, sizeof(srvaddr)) == -1)
        err_exit("bind");

    struct sockaddr_un claddr;
    socklen_t len = sizeof(claddr);
    char buf[BUF_SIZE];
    int nr = recvfrom(sfd, buf, sizeof(buf) - 1, 0, (struct sockaddr *)&claddr, &len);
    if (nr == -1)
        err_exit("recvfrom");
    buf[nr] = '\0';
    printf("Client sent: %s\n", buf);

    const char msg[] = "hi!";
    if (sendto(sfd, msg, sizeof(msg), 0, (struct sockaddr *)&claddr, sizeof(claddr)) != sizeof(msg))
        err_exit("sendto");
    
    if (close(sfd) == -1)
        err_exit("close");
    if (remove(srvaddr.sun_path) == -1)
        err_exit("remove");
    return EXIT_SUCCESS;
}
