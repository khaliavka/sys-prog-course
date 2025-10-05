#include <stddef.h>
#include "clients_t.h"

int add_client(clients_t *clients, uint32_t ipaddr, uint16_t port, client_t **client)
{
    if (clients->cl_count == CLIENTS_SIZE ||
        search_client(clients, ipaddr, port, NULL) != -1)
        return -1;
    clients->cls[clients->cl_count++] = (client_t){
        .ipaddr = ipaddr,
        .port = port,
        .msg_count = 1};
    if (client)
        *client = &clients->cls[clients->cl_count - 1];
    return 0;
}

int remove_client(clients_t *clients, int idx)
{
    clients->cls[idx] = clients->cls[clients->cl_count - 1];
    --clients->cl_count;
    return 0;
}

int search_client(clients_t *clients, uint32_t ipaddr, uint16_t port, client_t **client)
{
    for (int i = 0; i < clients->cl_count; ++i)
    {
        if (clients->cls[i].ipaddr == ipaddr && clients->cls[i].port == port)
        {
            if (client)
                *client = &clients->cls[i];
            return i;
        }
    }
    if (client)
        *client = NULL;
    return -1;
}