#include <errno.h>
#include <stdio.h>

#include "input.h"
#include "commands.h"

int read_line(char *line, int sz)
{
    printf("Command: ");
    if (fgets(line, sz, stdin) == NULL)
    {
        if (errno == EINTR)
            return -1;
        err_exit("fgets");
    }
    line[strcspn(line, "\n")] = '\0';
    return 0;
}

int parse_command(char *str, int sz, command_args_t *cmd_args)
{
    char *token = strtok(str, " ");
    if (strncmp(token, CREATE_DRIVER_S, sz) == 0)
    {
        cmd_args->cmd = CREATE_DRIVER;
        cmd_args->args.create_driver = (create_driver_t){};
    }
    else if (strncmp(token, SEND_TASK_S, sz) == 0)
    {
        cmd_args->cmd = SEND_TASK;
        cmd_args->args.send_task = (send_task_t){};
        token = strtok(NULL, " ");
        if (!token)
            return -1;
        sscanf(token, "%d", &cmd_args->args.send_task.driver_pid);
        token = strtok(NULL, " ");
        if (!token)
            return -1;
        sscanf(token, "%ld", &cmd_args->args.send_task.busy_secs);
    }
    else if (strncmp(token, GET_STATUS_S, sz) == 0)
    {
        cmd_args->cmd = GET_STATUS;
        cmd_args->args.get_status = (get_status_t){};
        token = strtok(NULL, " ");
        if (!token)
            return -1;
        sscanf(token, "%d", &cmd_args->args.get_status.driver_pid);
    }
    else if (strncmp(token, GET_DRIVERS_S, sz) == 0)
    {
        cmd_args->cmd = GET_DRIVERS;
        cmd_args->args.get_drivers = (get_drivers_t){};
    }
    else
    {
        return -1;
    }
    return 0;
}