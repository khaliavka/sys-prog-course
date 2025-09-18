#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include "exitmacro.h"
#include "settings.h"

int main(void)
{
    int sfd = socket(AF_LOCAL, SOCK_STREAM, 0);
    if (    sfd == -1)
        err_exit("socket");

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_LOCAL;
    strncpy(addr.sun_path, SRV_SONAME, sizeof(addr.sun_path) - 1);

    if (connect(sfd, (const struct sockaddr *)&addr, sizeof(addr)) == -1)
        err_exit("connect");
    const char *msg = "hello!";
    int len = strlen(msg);
    if (send(sfd, msg, len, 0) != len)
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
