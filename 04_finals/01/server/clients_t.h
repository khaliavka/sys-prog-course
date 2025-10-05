#ifndef CLIENTS_T
#define CLIENTS_T

#include <stdint.h>

#define CLIENTS_SIZE 1000

typedef struct
{
    uint32_t ipaddr;
    uint16_t port;
    int msg_count;
} client_t;

typedef struct
{
    client_t cls[CLIENTS_SIZE];
    int cl_count;
} clients_t;

int add_client(clients_t *clients, uint32_t ipaddr, uint16_t port, client_t **client);
int remove_client(clients_t *clients, int idx);
int search_client(clients_t *clients, uint32_t ipaddr, uint16_t port, client_t **client);

#endif
