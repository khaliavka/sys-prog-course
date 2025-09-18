#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "../exitmacro.h"
#include "../settings.h"

#define TTL 64

struct ipv4_header_t
{
    uint8_t version_ihl;
    uint8_t type_of_serv;
    uint16_t total_len;
    uint16_t id;
    uint16_t flags_frag_offset;
    uint8_t ttl;
    uint8_t protocol;
    uint16_t header_checksum;
    uint32_t src_ipaddr;
    uint32_t dst_ipaddr;
} __attribute__((packed));

int init_ipv4_header(struct ipv4_header_t *h, uint16_t total_len,
                     uint8_t protocol, const char *src_ipaddr, const char *dst_ipaddr)
{
    h->version_ihl = (4 << 4) | 5;
    h->type_of_serv = 0;
    h->total_len = htons(total_len);
    h->id = htons(0);
    h->flags_frag_offset = htons(0);
    h->ttl = TTL;
    h->protocol = protocol;
    h->header_checksum = htons(0);
    if (inet_pton(AF_INET, src_ipaddr, &h->src_ipaddr) != 1)
        err_exit("inet_pton");
    if (inet_pton(AF_INET, dst_ipaddr, &h->dst_ipaddr) != 1)
        err_exit("inet pton");
    return 0;
}

struct udp_header_t
{
    uint16_t src_port;
    uint16_t dst_port;
    uint16_t total_len;
    uint16_t checksum;
} __attribute__((packed));

int init_udp_header(struct udp_header_t *h, uint16_t src_port,
                    uint16_t dst_port, uint16_t total_len)
{
    h->src_port = htons(src_port);
    h->dst_port = htons(dst_port);
    h->total_len = htons(total_len);
    h->checksum = htons(0);
    return 0;
}

int main(void)
{
    char packetbuf[BUF_SIZE];
    struct ipv4_header_t ipv4_header;
    struct udp_header_t udp_header;
    const char msg[] = "-----------------------------";
    size_t ippacksz = sizeof(ipv4_header) + sizeof(udp_header) + sizeof(msg);

    init_ipv4_header(&ipv4_header, ippacksz, IPPROTO_UDP, CL_IPADDR, SRV_IPADDR);
    init_udp_header(&udp_header, CL_PORT, SRV_PORT, sizeof(udp_header) + sizeof(msg));

    memcpy(packetbuf, &ipv4_header, sizeof(ipv4_header));
    memcpy(packetbuf + sizeof(ipv4_header), &udp_header, sizeof(udp_header));
    memcpy(packetbuf + sizeof(ipv4_header) + sizeof(udp_header), msg, sizeof(msg));

    int sfd = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);
    if (sfd == -1)
        err_exit("socket");
    int option = 1;
    if (setsockopt(sfd, IPPROTO_IP, IP_HDRINCL, &option, sizeof(option)) == -1)
        err_exit("setsockopt");

    struct sockaddr_in srvaddr;
    memset(&srvaddr, 0, sizeof(srvaddr));
    srvaddr.sin_family = AF_INET;
    if (inet_pton(AF_INET, SRV_IPADDR, &srvaddr.sin_addr) != 1)
        err_exit("inet_pton");

    if (sendto(sfd, packetbuf, ippacksz, 0,
               (const struct sockaddr *)&srvaddr, sizeof(srvaddr)) != (ssize_t)ippacksz)
        err_exit("sendto");

    char buf[BUF_SIZE];
    
    while (1)
    {
        int nr = recvfrom(sfd, buf, sizeof(buf) - 1, 0, NULL, NULL);
        if (nr == -1)
            err_exit("recvfrom");
        buf[nr] = '\0';
        struct ipv4_header_t *iphdr = (struct ipv4_header_t *)buf;
        struct udp_header_t *udphdr = (struct udp_header_t *)(iphdr + 1);
        uint32_t income_dst_ipaddr = iphdr->dst_ipaddr;
        uint16_t income_dst_port = udphdr->dst_port;

        if (income_dst_ipaddr == ipv4_header.src_ipaddr && income_dst_port == udp_header.src_port)
        {
            printf("%s\n", (const char *)(udphdr + 1));
            break;
        }
    }
    if (close(sfd) == -1)
        err_exit("close");
    return EXIT_SUCCESS;
}
