#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <mqueue.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <unistd.h>

#include "gui.h"
#include "client.h"

static void sig_winch(int signo)
{
    (void)signo;
    struct winsize size;
    ioctl(fileno(stdout), TIOCGWINSZ, (char *)&size);
    resizeterm(size.ws_row, size.ws_col);
}

int draw_wnd(WINDOW **container, WINDOW **wnd, int y, int x, int rows, int cols, int color_pair)
{
    *container = newwin(rows + 2, cols + 2, y, x);
    wattron(*container, COLOR_PAIR(color_pair));
    wbkgd(*container, COLOR_PAIR(color_pair));
    *wnd = derwin(*container, rows, cols, 1, 1);
    box(*container, ACS_VLINE, ACS_HLINE);
    wrefresh(*container);
    wrefresh(*wnd);
    return 0;
}

int init_gui(void)
{
    initscr();
    signal(SIGWINCH, sig_winch);
    curs_set(FALSE);
    start_color();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    set_escdelay(250);
    refresh();
    init_pair(1, COLOR_WHITE, COLOR_BLUE);
    init_pair(2, COLOR_WHITE, COLOR_RED);
    return 0;
}

int draw_gui(struct windows_t *windows)
{
    draw_wnd(&windows->messages.container, &windows->messages.wnd, 0, 0, MSGWND_ROWS, MSGWND_COLS, 1);
    draw_wnd(&windows->users.container, &windows->users.wnd, 0, MSGWND_COLS + 2, USRWND_ROWS, USRWND_COLS, 1);
    draw_wnd(&windows->input.container, &windows->input.wnd, MSGWND_ROWS + 2, 0, INPUTWND_ROWS, INPUTWND_COLS, 1);
    print_message(windows->input.wnd, "", "Message");
    return 0;
}

int get_box_input(int y, int x, char *buf, int sz, char *prompt)
{
    WINDOW *container;
    WINDOW *wnd;
    draw_wnd(&container, &wnd, y, x, USERNAME_ROWS, USERNAME_COLS + 20, 2);
    print_message(wnd, "", prompt);
    read_input(wnd, buf, sz, prompt);
    delwin(wnd);
    delwin(container);
    return 0;
}

int refresh_windows(struct windows_t *win)
{
    werase(win->messages.container);
    werase(win->messages.wnd);
    werase(win->users.container);
    werase(win->users.wnd);
    werase(win->input.container);
    werase(win->input.wnd);
    box(win->messages.container, ACS_VLINE, ACS_HLINE);
    box(win->users.container, ACS_VLINE, ACS_HLINE);
    box(win->input.container, ACS_VLINE, ACS_HLINE);
    print_message(win->input.wnd, "", "Message");
    wrefresh(win->messages.container);
    wrefresh(win->messages.wnd);
    wrefresh(win->users.container);
    wrefresh(win->users.wnd);
    wrefresh(win->input.container);
    wrefresh(win->input.wnd);
    return 0;
}

int print_message(WINDOW *wnd, char *msg, char *prompt)
{
    werase(wnd);
    mvwprintw(wnd, 0, 0, "%s: %s", prompt, msg);
    wrefresh(wnd);
    return 0;
}

int print_line(WINDOW *wnd, int y, int x, char *buf)
{
    mvwprintw(wnd, y, x, "%s", buf);
    wrefresh(wnd);
    return 0;
}

int read_input(WINDOW *wnd, char *msg, int sz, char *prompt)
{
    int i = 0;
    int ch;
    while ((ch = getch()) != '\n')
    {
        switch (ch)
        {
        case KEY_BACKSPACE:
            if (i > 0)
            {
                msg[--i] = '\0';
            }
            break;
        default:
            if (i < sz - 1)
            {
                msg[i++] = (char)ch;
                msg[i] = '\0';
            }
            break;
        }
        print_message(wnd, msg, prompt);
    }
    return 0;
}

void cleanup_gui(struct windows_t *windows)
{
    delwin(windows->messages.container);
    delwin(windows->messages.wnd);
    delwin(windows->users.container);
    delwin(windows->users.wnd);
    delwin(windows->input.container);
    delwin(windows->input.wnd);
    endwin();
}
