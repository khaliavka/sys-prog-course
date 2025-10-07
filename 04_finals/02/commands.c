#include <unistd.h>
#include <sys/socket.h>

#include "exitmacro.h"
#include "commands.h"
#include "drivers_t.h"
#include "driver_main.h"
#include "message_t.h"

int create_driver(create_driver_t *args, drivers_t *drivers)
{
    int fds[2];
    if (socketpair(AF_UNIX, SOCK_DGRAM, 0, fds) == -1)
        err_exit("socketpair");

    pid_t cpid = fork();
    if (cpid == -1)
        err_exit("fork");

    if (cpid == 0)
    {
        close(fds[0]);
        driver_main(fds[1]);
        exit(EXIT_SUCCESS);
    }
    close(fds[1]);
    if (add_driver(drivers, cpid, fds[0]) == -1)
        return -1;

    return 0;
}

int send_task(send_task_t *args, driver_t *drivers)
{
    int fd = search_driver(drivers, args->driver_pid);
    if (fd == -1)
        return -1;
    message_t msg = {.msg_type = MSG_SEND_TASK, .args.busy_secs = args->busy_secs};
    if (write(fd, &msg, sizeof(msg)) == -1)
        err_exit("write_af_unix_sock");
    return 0;
}

int get_status(get_status_t *args, drivers_t *drivers)
{
    int fd = search_driver(drivers, args->driver_pid);
    if (fd == -1)
        return -1;
    message_t msg = {.msg_type = MSG_GET_STATUS};
    if (write(fd, &msg, sizeof(msg)) == -1)
        err_exit("write_af_unix_sock");
    return 0;
}

int get_drivers(get_drivers_t *args, drivers_t *drivers)
{

    return 0;
}