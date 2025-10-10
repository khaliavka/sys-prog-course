#include <sys/epoll.h>
#include <sys/timerfd.h>
#include <string.h>

#include "main_impl.h"
#include "exitmacro.h"

int epoll_add_fd(int epollfd, int fd)
{
    struct epoll_event ev;
    memset(&ev, 0, sizeof(ev));
    ev.events = EPOLLIN;
    ev.data.fd = fd;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ev) == -1)
        err_exit("epoll_ctl");
    return 0;
}

int epoll_del_fd(int epollfd, int fd)
{
    if (epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, NULL) == -1)
        err_exit("epoll_ctl");
    return 0;
}

int driver_setbusy(int tfd, time_t busy_sec)
{
    struct itimerspec tval;
    tval.it_value.tv_sec = busy_sec;
    tval.it_value.tv_nsec = 0;
    tval.it_interval.tv_sec = 0;
    tval.it_interval.tv_nsec = 0;
    if (timerfd_settime(tfd, 0, &tval, NULL) == -1)
        err_exit("timerfd_settime");
    return 0;
}

int driver_getbusy(int tfd, time_t *busy_sec)
{
    struct itimerspec tval;
    if (timerfd_gettime(tfd, &tval) == -1)
        err_exit("timerfd_gettime");
    *busy_sec = tval.it_value.tv_sec;
    return 0;
}