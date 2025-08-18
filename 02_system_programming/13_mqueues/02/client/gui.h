#ifndef GUI_H
#define  GUI_H

#include <curses.h>

#define USERNAME_ROWS 1
#define USERNAME_COLS 20

#define MSGWND_ROWS 20
#define MSGWND_COLS 100

#define USRWND_ROWS 20
#define USRWND_COLS 20

#define INPUTWND_ROWS 1
#define INPUTWND_COLS 122

struct window_t
{
    WINDOW *container;
    WINDOW *wnd;
};

struct windows_t
{
    struct window_t messages;
    struct window_t users;
    struct window_t input;

};

int init_gui(void);
void cleanup_gui(struct windows_t *windows);
int draw_gui(struct windows_t *windows);
int refresh_windows(struct windows_t *win);

int get_box_input(int y, int x, char *buf, int sz, char *prompt);
int read_input(WINDOW *wnd, char *msg, int sz, char *prompt);

int print_message(WINDOW *wnd, char *msg, char *prompt);
int print_line(WINDOW *wnd, int y, int x, char *buf);
int print_buffer(WINDOW *wnd, char *buf);

#endif