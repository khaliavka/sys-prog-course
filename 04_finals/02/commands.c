#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/epoll.h>

#include "commands.h"
#include "exitmacro.h"
#include "drivers_t.h"
#include "driver_main.h"
#include "message_t.h"
#include "main_impl.h"

int create_driver(create_driver_t *args, drivers_t *drivers, int epollfd)
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
        close(epollfd);
        cleanup_drivers(drivers, close);
        driver_main(fds[1]);
        exit(EXIT_SUCCESS);
    }

    close(fds[1]);
    if (add_driver(drivers, cpid, fds[0]) == -1)
        return -1;
    epoll_add_fd(epollfd, fds[0]);
    return 0;
}

int delete_driver(delete_driver_t *args, drivers_t *drivers, drivers_t *zombies, int epollfd)
{
    int index = search_driver_by_pid(drivers, args->driver_pid);
    if (index == -1)
        return -1;
    int pidfd = pidfd_open(drivers->arr[index].pid, 0);
    epoll_add_fd(epollfd, pidfd);
    add_driver(zombies, drivers->arr[index].pid, pidfd);

    message_t msg = {.msg_type = MSG_DEL, .args.del = (msg_del_t){}};
    if (write(drivers->arr[index].fd, &msg, sizeof(msg)) == -1)
        err_exit("write_af_unix_sock");
        
    epoll_del_fd(epollfd, drivers->arr[index].fd);
    close(drivers->arr[index].fd);
    remove_driver(drivers, index);
    return 0;
}

int send_task(send_task_t *args, drivers_t *drivers)
{
    int index = search_driver_by_pid(drivers, args->driver_pid);
    if (index == -1)
        return -1;
    message_t msg = {.msg_type = MSG_SEND_TASK, .args.send_task = (msg_send_task_t){.busy_secs = args->busy_secs}};
    if (write(drivers->arr[index].fd, &msg, sizeof(msg)) == -1)
        err_exit("write_af_unix_sock");
    return 0;
}

int get_status(get_status_t *args, drivers_t *drivers)
{
    int index = search_driver_by_pid(drivers, args->driver_pid);
    if (index == -1)
        return -1;
    message_t msg = {.msg_type = MSG_GET_STATUS, .args.get_status = (msg_get_status_t){}};
    if (write(drivers->arr[index].fd, &msg, sizeof(msg)) == -1)
        err_exit("write_af_unix_sock");
    return 0;
}

int get_drivers(get_drivers_t *args, drivers_t *drivers)
{
    (void)*args;
    if (drivers->count == 0)
        return -1;

    for (int i = 0; i < drivers->count; ++i)
    {
        message_t msg = {.msg_type = MSG_GET_STATUS, .args.get_status = (msg_get_status_t){}};
        if (write(drivers->arr[i].fd, &msg, sizeof(msg)) == -1)
            err_exit("write_af_unix_sock");
    }
    return 0;
}

int exec_command(command_args_t *cmd_args, drivers_t *drivers, drivers_t *zombies, int epollfd)
{
    switch (cmd_args->cmd)
    {
    case CREATE_DRIVER:
        if (create_driver(&cmd_args->args.create_driver, drivers, epollfd) == -1)
        {
            puts("Cannot create driver.");
            return -1;
        }
        break;
    case DELETE_DRIVER:
        if (delete_driver(&cmd_args->args.delete_driver, drivers, zombies, epollfd) == -1)
        {
            puts("Cannot delete driver (wrong pid).");
            return -1;
        }
        break;
    case SEND_TASK:
        if (send_task(&cmd_args->args.send_task, drivers) == -1)
        {
            puts("Wrong pid.");
            return -1;
        }
        break;
    case GET_STATUS:
        if (get_status(&cmd_args->args.get_status, drivers) == -1)
        {
            puts("Wrong pid.");
            return -1;
        }
        break;
    case GET_DRIVERS:
        if (get_drivers(&cmd_args->args.get_drivers, drivers) == -1)
            return -1;
        break;
    default:
        break;
    }
    return 0;
}