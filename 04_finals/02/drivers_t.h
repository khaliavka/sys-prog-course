#ifndef DRIVERS_T
#define DRIVERS_T

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
int linsearch_driver(drivers_t *drivers, pid_t pid);
int remove_driver(drivers_t *drivers, int index, int (*do_cleanup)(int));

#endif
