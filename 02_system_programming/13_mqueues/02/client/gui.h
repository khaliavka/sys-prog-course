#ifndef GUI_H
#define  GUI_H

#include <curses.h>

#define NICKNAME_ROWS 1
#define NICKNAME_COLS 20

#define MSGWND_ROWS 20
#define MSGWND_COLS 100

#define USRWND_ROWS 20
#define USRWND_COLS 20

#define INPUTWND_ROWS 1
#define INPUTWND_COLS 122

struct window
{
    WINDOW *container;
    WINDOW *wnd;
};

struct windows
{
    struct window messages;
    struct window users;
    struct window input;

};

int init_gui(void);
void cleanup_gui(struct windows *windows);
int draw_gui(struct windows *windows);
int refresh_windows(struct windows *win);

int get_box_input(int y, int x, char *buf, int sz, char *prompt);
int read_input(WINDOW *wnd, char *msg, int sz, char *prompt);

int print_message(WINDOW *wnd, char *msg, char *prompt);
int print_line(WINDOW *wnd, int y, int x, char *buf);

#endif