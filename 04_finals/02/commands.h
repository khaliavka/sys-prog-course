#ifndef COMMANDS_H
#define COMMANDS_H

#include <sys/types.h>

#include "drivers_t.h"

#define CREATE_DRIVER_S "create_driver"
#define DELETE_DRIVER_S "delete_driver"
#define SEND_TASK_S "send_task"
#define GET_STATUS_S "get_status"
#define GET_DRIVERS_S "get_drivers"

typedef enum
{
    CREATE_DRIVER,
    DELETE_DRIVER,
    SEND_TASK,
    GET_STATUS,
    GET_DRIVERS
} command_t;

typedef struct {} create_driver_t;
typedef struct {} get_drivers_t;

typedef struct
{
    pid_t driver_pid;
} delete_driver_t;

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
        delete_driver_t delete_driver;
        send_task_t send_task;
        get_status_t get_status;
        get_drivers_t get_drivers;

    } args;
    
} command_args_t;

int create_driver(create_driver_t *args, drivers_t *drivers, int epollfd);
int delete_driver(delete_driver_t *args, drivers_t *drivers, int epollfd);
int send_task(send_task_t *args, drivers_t *drivers);
int get_status(get_status_t *args, drivers_t *drivers);
int get_drivers(get_drivers_t *args, drivers_t *drivers);
int exec_command(command_args_t *cmd_args, drivers_t *drivers, int epollfd);

#endif