#ifndef PROC_BUFFER_H
#define PROC_BUFFER_H

#include <linux/mutex.h>

typedef struct
{
    char *bufp;
    size_t len;
    size_t capacity;
    struct mutex mtx;

} buffer_t;

#endif