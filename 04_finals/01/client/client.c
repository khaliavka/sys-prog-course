#include <errno.h>
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
#include <stdatomic.h>

#include "../exitmacro.h"
#include "../settings.h"
#include "../commands.h"
#include "proto_headers.h"

atomic_int cancel_flag = 0;

void sigint_handler(int sig)
{
    (void)sig;
    cancel_flag = 1;
}

int read_line(char *line, int sz)
{
    printf("Message: ");
    if (fgets(line, sz, stdin) == NULL)
    {
        if (errno == EINTR)
            return -1;
        err_exit("fgets");
    }
    line[strcspn(line, "\n")] = '\0';
    return 0;
}

int send_message(int sfd, char *packbuf, const char *msg, ipv4_header_t *iphdr,
                 udp_header_t *udphdr, const struct sockaddr_ll *dst_addrp)
{
    size_t udp_total_len = sizeof(udp_header_t) + strlen(msg) + 1;
    size_t ip_total_len = sizeof(ipv4_header_t) + udp_total_len;
    size_t eth_total_len = sizeof(eth_header_t) + ip_total_len;
    set_ipv4_h_total_len(iphdr, ip_total_len);
    set_ipv4_h_checksum(iphdr);
    set_udp_h_total_len(udphdr, udp_total_len);
    if (sendto(sfd, packbuf, eth_total_len, 0, (const struct sockaddr *)dst_addrp,
               sizeof(struct sockaddr_ll)) != (ssize_t)eth_total_len)
        err_exit("sendto");
    return 0;
}

int main(void)
{
    struct sigaction sa = {.sa_handler = sigint_handler, .sa_flags = 0};
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIGINT, &sa, NULL) == -1)
        err_exit("sigaction");

    char out_packetbuf[MAXPACK_SIZE];
    eth_header_t *const out_eth_hdr = (eth_header_t *)out_packetbuf;
    ipv4_header_t *const out_ipv4_hdr = (ipv4_header_t *)(out_eth_hdr + 1);
    udp_header_t *const out_udp_hdr = (udp_header_t *)(out_ipv4_hdr + 1);
    char *const out_msg = (char *const)(out_udp_hdr + 1);

    char in_packetbuf[MAXPACK_SIZE];
    const eth_header_t *const in_eth_hdr = (const eth_header_t *)in_packetbuf;
    const ipv4_header_t *const in_ipv4_hdr = (const ipv4_header_t *)(in_eth_hdr + 1);
    const udp_header_t *const in_udp_hdr = (const udp_header_t *)(in_ipv4_hdr + 1);
    const char *const in_msg = (const char *const)(in_udp_hdr + 1);

    init_eth_header(out_eth_hdr, SRV_MAC, CL_MAC, MACADDR_LEN);
    init_ipv4_header(out_ipv4_hdr, 0, IPPROTO_UDP, CL_IPADDR, SRV_IPADDR, TTL);
    init_udp_header(out_udp_hdr, CL_PORT, SRV_PORT, 0);

    struct sockaddr_ll dst_addr;
    memset(&dst_addr, 0, sizeof(dst_addr));
    dst_addr.sll_family = AF_PACKET;
    int sll_ifindex = if_nametoindex(CL_IFNAME);
    if (sll_ifindex == 0)
        err_exit("if_nametoindex");
    dst_addr.sll_ifindex = sll_ifindex;
    dst_addr.sll_halen = MACADDR_LEN;
    for (int i = 0; i < MACADDR_LEN; ++i)
        dst_addr.sll_addr[i] = out_eth_hdr->dst_mac[i];

    int sfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (sfd == -1)
        err_exit("socket");

    while (cancel_flag == 0)
    {
        if (read_line(out_msg, MAXMSG_SIZE) == -1)
            continue;
        send_message(sfd, out_packetbuf, out_msg, out_ipv4_hdr, out_udp_hdr, &dst_addr);

        while (cancel_flag == 0)
        {
            int nr = recvfrom(sfd, in_packetbuf, sizeof(in_packetbuf) - 1, 0, NULL, NULL);
            if (nr == -1)
            {
                if (errno == EINTR)
                    continue;
                err_exit("recvfrom");
            }
            in_packetbuf[nr] = '\0';

            uint32_t in_dst_ipaddr = in_ipv4_hdr->dst_ipaddr;
            uint16_t in_dst_port = in_udp_hdr->dst_port;

            if (is_same_macaddr(in_eth_hdr->dst_mac, out_eth_hdr->src_mac, MACADDR_LEN) &&
                in_dst_ipaddr == out_ipv4_hdr->src_ipaddr &&
                in_dst_port == out_udp_hdr->src_port)
            {
                printf("%s\n", in_msg);
                break;
            }
        }
    }
    strncpy(out_msg, CMD_EXIT, MAXMSG_SIZE);
    send_message(sfd, out_packetbuf, out_msg, out_ipv4_hdr, out_udp_hdr, &dst_addr);

    if (close(sfd) == -1)
        err_exit("close");
    return EXIT_SUCCESS;
}
