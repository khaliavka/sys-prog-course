#include <arpa/inet.h>

#include "proto_headers.h"
#include "../exitmacro.h"

int init_eth_header(struct eth_header_t *h, uint64_t dst_mac, uint64_t src_mac, int mac_len)
{
    for (int i = 0; i < mac_len; ++i)
    {
        h->dst_mac[i] = *((uint8_t *)&dst_mac + mac_len - 1 - i);
        h->src_mac[i] = *((uint8_t *)&src_mac + mac_len - 1 - i);
    }
    h->ethertype = htons(0x800);
    return 0;
}

int is_same_macaddr(uint8_t *l, uint8_t *r, int mac_len)
{
    for (int i = 0; i < mac_len; ++i)
    {
        if (l[i] != r[i])
            return 0;
    }
    return 1;
}

int init_ipv4_header(struct ipv4_header_t *h, uint16_t total_len,
                     uint8_t protocol, const char *src_ipaddr, const char *dst_ipaddr, uint8_t ttl)
{
    h->version_ihl = (4 << 4) | 5;
    h->type_of_serv = 0;
    h->total_len = htons(total_len);
    h->id = htons(0);
    h->flags_frag_offset = htons(0);
    h->ttl = ttl;
    h->protocol = protocol;
    if (inet_pton(AF_INET, src_ipaddr, &h->src_ipaddr) != 1)
        err_exit("inet_pton");
    if (inet_pton(AF_INET, dst_ipaddr, &h->dst_ipaddr) != 1)
        err_exit("inet pton");
    
    h->header_checksum = htons(0);
    h->header_checksum = calc_ipv4_header_checksum(h);
    return 0;
}

uint16_t calc_ipv4_header_checksum(struct ipv4_header_t *h)
{
    uint64_t sum = 0;

    for (int i = 0; i < 10; ++i)
        sum += ((uint16_t *)h)[i];

    sum += (sum >> 16);
    return ~(uint16_t)(0xffff & sum);
}

int init_udp_header(struct udp_header_t *h, uint16_t src_port,
                    uint16_t dst_port, uint16_t total_len)
{
    h->src_port = htons(src_port);
    h->dst_port = htons(dst_port);
    h->total_len = htons(total_len);
    h->checksum = htons(0);
    return 0;
}

/*

int test_checksum(void)
{
    // uint8_t ip_header[20] = {0x45, 0, 0, 0x73, 0, 0, 0x40, 0, 0x40, 0x11, 0, 0,
    //  0xc0, 0xa8, 0, 1, 0xc0, 0xa8, 0, 0xc7};
    uint8_t ip_header[20] = {0x45, 0, 0, 0x3a, 0, 0, 0, 0, 0x40, 0x11, 0, 0,
                             0xa, 0x21, 0x4d, 5, 0xa, 0x21, 0x4d, 0xc};
    uint16_t checksum = calc_ipv4_header_checksum((struct ipv4_header_t *)&ip_header);
    *(uint16_t *)(ip_header + 10) = checksum;
    checksum = calc_ipv4_header_checksum((struct ipv4_header_t *)&ip_header);
    return checksum == 0;
}

*/