#include "fdbuf_t.h"

int add_fd(fdbuf_t *fdbuf, int fd, fdtype_t fdtype)
{
    if (fdbuf->count == FDBUF_SIZE)
        return -1;
    fdbuf->fds[fdbuf->count++] = (fddata_t){.fd = fd, .type = fdtype};
    return 0;
}

int remove_fd(fdbuf_t *fdbuf, int fd, int (*do_cleanup)(int))
{
    for (int i = 0; i < fdbuf->count; ++i)
    {
        if (fdbuf->fds[i].fd == fd)
        {
            fdbuf->fds[i] = fdbuf->fds[fdbuf->count - 1];
            --fdbuf->count;
            do_cleanup(fd);
            break;
        }
    }
    return 0;
}
