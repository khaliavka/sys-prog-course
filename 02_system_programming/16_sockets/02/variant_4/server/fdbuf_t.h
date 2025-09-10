#ifndef FDBUF_T
#define FDBUF_T

#define FDBUF_SIZE 1000

typedef enum
{
    UDP,
    LISTEN_TCP,
    ACTIVE_TCP
} fdtype_t;

typedef struct
{
    int fd;
    fdtype_t type;
} fddata_t;

typedef struct
{
    fddata_t fds[FDBUF_SIZE];
    int count;
} fdbuf_t;

int add_fd(fdbuf_t *fdbuf, int fd, fdtype_t fdtype);
int remove_fd(fdbuf_t *fdbuf, int fd, int (*do_cleanup)(int));

#endif
