#ifndef GUI_H
#define  GUI_H

#include <curses.h>

#define MSGWND_ROWS 20
#define MSGWND_COLS 80
#define USRWND_ROWS 20
#define USRWND_COLS 20
#define ENTRWND_ROWS 3
#define ENTRWND_COLS 102


struct messages_wnd
{
    WINDOW *container;
    WINDOW *wnd; 

    char **msgs;
    int sz;
};

struct users_wnd
{
    WINDOW *container;
    WINDOW *wnd;

    char **users;
    int sz;
};

struct enter_wnd
{
    WINDOW *container;
    WINDOW *wnd;

    char *msg;
};


struct state
{
    struct messages_wnd messagesw;
    struct users_wnd usersw;
    struct enter_wnd enterw;
};

int init_gui(struct state *st);
void cleanup(struct state *st);
int read_message(WINDOW *wnd);

#endif