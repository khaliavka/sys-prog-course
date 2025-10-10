#ifndef DRIVER_MAIN_H
#define DRIVER_MAIN_H

#define DRIVER_MAX_EV 2

typedef enum
{
    STATE_AVAIL,
    STATE_BUSY
} driver_state_t;

int driver_main(int fd);

#endif