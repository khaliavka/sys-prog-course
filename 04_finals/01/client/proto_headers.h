#ifndef PROTO_HEADERS_H
#define PROTO_HEADERS_H

#include <stdint.h>

#define MACADDR_LEN 6
#define TTL 64
#define MAXPACK_SIZE 200
#define MAX_NUMLEN 10
#define MAXMSG_SIZE (MAXPACK_SIZE - MAX_NUMLEN - sizeof(eth_header_t) - \
    sizeof(ipv4_header_t) - sizeof(udp_header_t))

typedef struct 
{
    uint8_t dst_mac[MACADDR_LEN];
    uint8_t src_mac[MACADDR_LEN];
    uint16_t ethertype;
} __attribute__((packed)) eth_header_t;

typedef struct 
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
} __attribute__((packed)) ipv4_header_t;

typedef struct 
{
    uint16_t src_port;
    uint16_t dst_port;
    uint16_t total_len;
    uint16_t checksum;
} __attribute__((packed)) udp_header_t;

int init_eth_header(eth_header_t *h, uint64_t dst_mac, uint64_t src_mac, int mac_len);
int is_same_macaddr(const uint8_t *l, const uint8_t *r, int mac_len);
int init_ipv4_header(ipv4_header_t *h, uint16_t total_len,
    uint8_t protocol, const char *src_ipaddr, const char *dst_ipaddr, uint8_t ttl);
int set_ipv4_h_total_len(ipv4_header_t *h, uint16_t total_len);
int set_ipv4_h_checksum(ipv4_header_t *h);
int init_udp_header(udp_header_t *h, uint16_t src_port,
                    uint16_t dst_port, uint16_t total_len);
int set_udp_h_total_len(udp_header_t *h, uint16_t total_len);
// int test_checksum(void);

#endif