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

#define MSG_BUFSZ 281

static void sig_winch(int signo)
{
    (void)signo;
    struct winsize size;
    ioctl(fileno(stdout), TIOCGWINSZ, (char *)&size);
    resizeterm(size.ws_row, size.ws_col);
}

static int draw_wnd(WINDOW **container, WINDOW **wnd, int y, int x, int rows, int cols, int color_pair)
{
    *container = newwin(rows + 2, cols + 2, y, x);
    wattron(*container, COLOR_PAIR(color_pair));
    wbkgd(*container, COLOR_PAIR(color_pair));
    *wnd = derwin(*container, rows, cols, 1, 1);
    box(*container, ACS_VLINE, ACS_HLINE);

    // mvwprintw(p->panel, i - begin, 0, pattern, item->d_name);
    wrefresh(*container);
    wrefresh(*wnd);
    return 0;
}

int init_gui(struct state *st)
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
    init_pair(2, COLOR_BLACK, COLOR_WHITE);

    draw_wnd(&st->messagesw.container, &st->messagesw.wnd, 0, 0, MSGWND_ROWS, MSGWND_COLS, 1);
    draw_wnd(&st->usersw.container, &st->usersw.wnd, 0, MSGWND_COLS + 2, USRWND_ROWS, USRWND_COLS, 1);
    draw_wnd(&st->enterw.container, &st->enterw.wnd, MSGWND_ROWS + 2, 0, ENTRWND_ROWS, ENTRWND_COLS, 1);
    return 0;
}

int read_message(WINDOW *wnd)
{
    mvwprintw(wnd, 0, 0, "Enter text: ");
    wmove(wnd, 0, 13);
    wrefresh(wnd);
    char msg[MSG_BUFSZ] = {0};
    int ch;
    int i = 0;
    while ((ch = getch()) != '\n')
    {
        switch (ch)
        {
        case KEY_BACKSPACE:
            if (i > 0)
                msg[--i] = '\0';
            break;
        default:
            if (i < MSG_BUFSZ - 1)
                msg[i++] = (char)ch;
            break;
        }
        werase(wnd);
        mvwprintw(wnd, 0, 0, "Enter text: ");
        wmove(wnd, 0, 13);
        mvwprintw(wnd, 0, 13, "%s", msg);
        wrefresh(wnd);
    }
    return 0;
}

void cleanup(struct state *st)
{
    delwin(st->messagesw.container);
    delwin(st->messagesw.wnd);
    delwin(st->usersw.container);
    delwin(st->usersw.wnd);
    delwin(st->enterw.container);
    delwin(st->enterw.wnd);
    endwin();
}

// int render_panel(struct panel *p, int is_focus)
// {
//     werase(p->panel);

//     int begin = p->top;
//     int end = ((p->sz < begin + PANEL_ROWS) ? p->sz : begin + PANEL_ROWS);
//     int selected = p->selected;

//     for (int i = begin; i < end; ++i)
//     {
//         struct dirent *item = p->namelist[i];
//         char *pattern = ((item->d_type == DT_DIR ||
//                           item->d_type == DT_LNK)
//                              ? "/%s"
//                              : "%s");
//         int cp = ((is_focus && i == selected) ? 2 : 1);

//         wattron(p->panel, COLOR_PAIR(cp));
//         mvwprintw(p->panel, i - begin, 0, pattern, item->d_name);
//         wattroff(p->panel, COLOR_PAIR(cp));
//     }
//     wrefresh(p->panel);
//     return 0;
// }
