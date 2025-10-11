#ifndef MAIN_IMPL_H
#define MAIN_IMPL_H

#include <sys/types.h>

int epoll_add_fd(int epollfd, int fd);
int epoll_del_fd(int epollfd, int fd);
int pidfd_open(pid_t pid, unsigned int flags);
int driver_setbusy(int tfd, time_t busy_sec);
int driver_getbusy(int tfd, time_t *busy_sec);

#endif