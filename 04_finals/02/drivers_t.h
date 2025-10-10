#ifndef DRIVERS_T_H
#define DRIVERS_T_H

#include <unistd.h>

#define DRIVERS_COUNT 1000

typedef struct
{
    pid_t pid;
    int fd;
} driver_t;

typedef struct
{
    driver_t arr[DRIVERS_COUNT];
    int count;
} drivers_t;

int add_driver(drivers_t *drivers, pid_t pid, int fd);
int search_driver_by_pid(drivers_t *drivers, pid_t pid);
int search_driver_by_fd(drivers_t *drivers, int fd);
int remove_driver(drivers_t *drivers, int index, int (*do_cleanup)(int));
int cleanup_drivers(drivers_t *drivers, int (*do_cleanup)(int));

#endif
