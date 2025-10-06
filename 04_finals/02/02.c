#include <errno.h>
#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>

#include "exitmacro.h"
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

atomic_int cancel_flag = 0;

void sigint_handler(int sig)
{
    (void)sig;
    cancel_flag = 1;
}

int main(void)
{
    struct sigaction sa = {.sa_handler = sigint_handler, .sa_flags = 0};
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIGINT, &sa, NULL) == -1)
        err_exit("sigaction");

    char inputbuf[INPUTBUF_SIZE];
    command_args_t cmd_args;
    while (cancel_flag == 0)
    {
        if (read_line(inputbuf, sizeof(inputbuf)) == -1)
            continue;

        if (parse_command(inputbuf, sizeof(inputbuf), &cmd_args) == -1)
        {
            puts("Bad command.");
            continue;
        }

        switch (cmd_args.cmd)
        {
        case CREATE_DRIVER:
            puts("You entered 'create_driver'");
            break;
        case SEND_TASK:
            printf("You entered 'send_task %d %ld'\n",
                    cmd_args.args.send_task.driver_pid,
                    cmd_args.args.send_task.busy_secs);
            break;
        case GET_STATUS:
            printf("You entered 'get_status %d'\n",
                    cmd_args.args.get_status.driver_pid);
            break;
        case GET_DRIVERS:
            puts("You entered 'get_drivers'");
            break;
        default:
            break;
        }
    }
    return EXIT_SUCCESS;
}
