#include <signal.h>
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
    const char msg[] = "-----------------------------";
    size_t udpheadsz = 8;
    size_t udppacksz = sizeof(msg) + udpheadsz;
    uint16_t header[] = {htons(CL_PORT), htons(SRV_PORT),
                         htons(udppacksz), htons(0)};
    char packetbuf[BUF_SIZE];
    memcpy(packetbuf, header, udpheadsz);
    memcpy(packetbuf + udpheadsz, msg, sizeof(msg));

    int sfd = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);
    if (sfd == -1)
        err_exit("socket");

    struct sockaddr_in srvaddr;
    memset(&srvaddr, 0, sizeof(srvaddr));
    srvaddr.sin_family = AF_INET;
    if (inet_pton(AF_INET, SRV_IPADDR, &srvaddr.sin_addr) != 1)
        err_exit("inet_pton");

    if (sendto(sfd, packetbuf, udppacksz, 0,
               (const struct sockaddr *)&srvaddr, sizeof(srvaddr)) != (ssize_t)udppacksz)
        err_exit("sendto");

    char buf[BUF_SIZE];
    while (1)
    {
        int nr = recvfrom(sfd, buf, sizeof(buf) - 1, 0, NULL, NULL);
        if (nr == -1)
            err_exit("recvfrom");
        buf[nr] = '\0';
        uint16_t dst_port = ntohs(*(uint16_t *)(buf + 0x16));
        if (dst_port == CL_PORT)
        {
            printf("%s\n", (buf + 0x1c));
            break;
        }
    }
    if (close(sfd) == -1)
        err_exit("close");
    return EXIT_SUCCESS;
}
