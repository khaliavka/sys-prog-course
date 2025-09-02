#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include "exitmacro.h"
#include "srvsoname.h"

#define BACKLOG 1
#define BUF_SIZE 100

int main(void)
{
    int listenfd = socket(AF_LOCAL, SOCK_STREAM, 0);
    if (listenfd == -1)
        err_exit("socket");

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_LOCAL;
    strncpy(addr.sun_path, SRVSONAME, sizeof(addr.sun_path) - 1);

    if (bind(listenfd, (const struct sockaddr *)&addr, sizeof(addr)) == -1)
        err_exit("bind");
    if (listen(listenfd, BACKLOG) == -1)
        err_exit("listen");

    int acceptfd = accept(listenfd, NULL, NULL);
    if (acceptfd == -1)
        err_exit("accept");

    char buf[BUF_SIZE];
    int nr = recv(acceptfd, buf, sizeof(buf) - 1, 0);
    if (nr == -1)
        err_exit("recv");
    buf[nr] = '\0';
    printf("Client sent: %s\n", buf);

    const char *msg = "hi!";
    int len = strlen(msg);
    if (send(acceptfd, msg, len, 0) != len)
        err_exit("send");

    if (close(listenfd) == -1)
        err_exit("close");
    if (close(acceptfd) == -1)
        err_exit("close");
    if (remove(addr.sun_path) == -1)
        err_exit("remove");
    return EXIT_SUCCESS;
}
