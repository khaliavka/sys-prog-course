#ifndef PROTO_HEADERS_H
#define PROTO_HEADERS_H

#include <stdint.h>

#define MACADDR_LEN 6
#define TTL 64

struct eth_header_t
{
    uint8_t dst_mac[MACADDR_LEN];
    uint8_t src_mac[MACADDR_LEN];
    uint16_t ethertype;
} __attribute__((packed));

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

struct udp_header_t
{
    uint16_t src_port;
    uint16_t dst_port;
    uint16_t total_len;
    uint16_t checksum;
} __attribute__((packed));

int init_eth_header(struct eth_header_t *h, uint64_t dst_mac, uint64_t src_mac, int mac_len);
int is_same_macaddr(uint8_t *l, uint8_t *r, int mac_len);
int init_ipv4_header(struct ipv4_header_t *h, uint16_t total_len,
    uint8_t protocol, const char *src_ipaddr, const char *dst_ipaddr, uint8_t ttl);
uint16_t calc_ipv4_header_checksum(struct ipv4_header_t *h);
int init_udp_header(struct udp_header_t *h, uint16_t src_port,
                    uint16_t dst_port, uint16_t total_len);

// int test_checksum(void);

#endif