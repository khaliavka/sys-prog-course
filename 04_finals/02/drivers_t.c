#include "drivers_t.h"

int add_driver(drivers_t *drivers, pid_t pid, int fd)
{
    if (drivers->count == DRIVERS_COUNT)
        return -1;
    drivers->arr[drivers->count++] = (driver_t){.pid = pid, .fd = fd};
    return 0;
}

int search_driver_by_pid(drivers_t *drivers, pid_t pid)
{
    for (int i = 0; i < drivers->count; ++i)
        if (drivers->arr[i].pid == pid)
            return i;
    return -1;
}

int search_driver_by_fd(drivers_t *drivers, int fd)
{
    for (int i = 0; i < drivers->count; ++i)
        if (drivers->arr[i].fd == fd)
            return i;
    return -1;
}

int remove_driver(drivers_t *drivers, int index)
{
    drivers->arr[index] = drivers->arr[drivers->count - 1];
    --drivers->count;
    return 0;
}

int cleanup_drivers(drivers_t *drivers, int (*do_cleanup)(int))
{
    for (int i = 0; i < drivers->count; ++i)
        do_cleanup(drivers->arr[i].fd);
    return 0;
}