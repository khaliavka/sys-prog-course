#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>
#include <net/ethernet.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "../exitmacro.h"
#include "../settings.h"
#include "proto_headers.h"

int main(void)
{
    char packetbuf[BUF_SIZE];
    struct eth_header_t eth_header;
    struct ipv4_header_t ipv4_header;
    struct udp_header_t udp_header;
    const char msg[] = "-----------------------------";
    size_t ip_pack_sz = sizeof(ipv4_header) + sizeof(udp_header) + sizeof(msg);
    size_t eth_pack_sz = sizeof(eth_header) + ip_pack_sz;

    init_ipv4_header(&ipv4_header, ip_pack_sz, IPPROTO_UDP, CL_IPADDR, SRV_IPADDR, TTL);
    init_udp_header(&udp_header, CL_PORT, SRV_PORT, sizeof(udp_header) + sizeof(msg));
    init_eth_header(&eth_header, SRV_MAC, CL_MAC, MACADDR_LEN);

    memcpy(packetbuf, &eth_header, sizeof(eth_header));
    memcpy(packetbuf + sizeof(eth_header), &ipv4_header, sizeof(ipv4_header));
    memcpy(packetbuf + sizeof(eth_header) + sizeof(ipv4_header), &udp_header, sizeof(udp_header));
    memcpy(packetbuf + sizeof(eth_header) + sizeof(ipv4_header) + sizeof(udp_header), msg, sizeof(msg));

    int sfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (sfd == -1)
        err_exit("socket");

    struct sockaddr_ll dst_addr;
    memset(&dst_addr, 0, sizeof(dst_addr));
    dst_addr.sll_family = AF_PACKET;
    int sll_ifindex = if_nametoindex(CL_IFNAME);
    if (sll_ifindex == 0)
        err_exit("if_nametoindex");
    dst_addr.sll_ifindex = sll_ifindex;
    dst_addr.sll_halen = MACADDR_LEN;
    for (int i = 0; i < MACADDR_LEN; ++i)
        dst_addr.sll_addr[i] = eth_header.dst_mac[i];

    if (sendto(sfd, packetbuf, eth_pack_sz, 0,
               (const struct sockaddr *)&dst_addr, sizeof(dst_addr)) != (ssize_t)eth_pack_sz)
        err_exit("sendto");

    char buf[BUF_SIZE];
    while (1)
    {
        int nr = recvfrom(sfd, buf, sizeof(buf) - 1, 0, NULL, NULL);
        if (nr == -1)
            err_exit("recvfrom");
        buf[nr] = '\0';
        struct eth_header_t *ethhdr = (struct eth_header_t *)buf;
        struct ipv4_header_t *iphdr = (struct ipv4_header_t *)(ethhdr + 1);
        struct udp_header_t *udphdr = (struct udp_header_t *)(iphdr + 1);

        uint32_t income_dst_ipaddr = iphdr->dst_ipaddr;
        uint16_t income_dst_port = udphdr->dst_port;

        if (is_same_macaddr(ethhdr->dst_mac, eth_header.src_mac, MACADDR_LEN) &&
            income_dst_ipaddr == ipv4_header.src_ipaddr &&
            income_dst_port == udp_header.src_port)
        {
            printf("%s\n", (const char *)(udphdr + 1));
            break;
        }
    }
    if (close(sfd) == -1)
        err_exit("close");
    return EXIT_SUCCESS;
}
