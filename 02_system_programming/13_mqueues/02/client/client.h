#ifndef CLIENT_H
#define CLIENT_H

#include <ncurses.h>

struct args_t
{
    WINDOW *wnd;
    mqd_t mqd;
};

#endif