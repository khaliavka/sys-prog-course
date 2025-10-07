#include "drivers_t.h"

int add_driver(drivers_t *drivers, pid_t pid, int fd)
{
    if (drivers->count == DRIVERS_COUNT)
        return -1;
    drivers->arr[drivers->count++] = (driver_t){.pid = pid, .fd = fd};
    return 0;
}

int linsearch_driver(drivers_t *drivers, pid_t pid)
{
    for (int i = 0; i < drivers->count; ++i)
        if (drivers->arr[i].pid == pid)
            return i;
    return -1;
}

int remove_driver(drivers_t *drivers, int index, int (*do_cleanup)(int))
{
    do_cleanup(drivers->arr[index].fd);
    drivers->arr[index] = drivers->arr[drivers->count - 1];
    --drivers->count;
    return 0;
}
