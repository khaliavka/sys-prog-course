#ifndef CLIENT_H
#define CLIENT_H

#include <ncurses.h>

#include "shmbuf_t.h"

struct args_t
{
    WINDOW *wnd;
    struct shmbuf_t *shmp;
};

#endif