#ifndef COMMANDS_H
#define COMMANDS_H

#include <sys/types.h>

#define INPUTBUF_SIZE 100

#define CREATE_DRIVER_S "create_driver"
#define SEND_TASK_S "send_task"
#define GET_STATUS_S "get_status"
#define GET_DRIVERS_S "get_drivers"

typedef enum
{
    CREATE_DRIVER,
    SEND_TASK,
    GET_STATUS,
    GET_DRIVERS
} command_t;

typedef struct {} create_driver_t;
typedef struct {} get_drivers_t;

typedef struct
{
    pid_t driver_pid;
    time_t busy_secs;
} send_task_t;

typedef struct
{
    pid_t driver_pid;
} get_status_t;

typedef struct
{
    command_t cmd;
    union
    {
        create_driver_t create_driver;
        send_task_t send_task;
        get_status_t get_status;
        get_drivers_t get_drivers;

    } args;
    
} command_args_t;

#endif