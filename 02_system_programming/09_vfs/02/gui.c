#include "gui.h"
#include "main.h"

void sig_winch(int signo)
{
    (void)signo;
    struct winsize size;
    ioctl(fileno(stdout), TIOCGWINSZ, (char *)&size);
    resizeterm(size.ws_row, size.ws_col);
}

int init_gui(struct state *state, void (*handler)(int))
{
    WINDOW *containers[NPANELS];
    WINDOW *panels[NPANELS];

    initscr();
    signal(SIGWINCH, handler);
    curs_set(FALSE);
    start_color();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    set_escdelay(250);
    refresh();
    init_pair(1, COLOR_WHITE, COLOR_BLUE);
    init_pair(2, COLOR_BLACK, COLOR_CYAN);

    for (int i = 0; i < NPANELS; ++i)
    {
        containers[i] = newwin(PANEL_ROWS + 2, PANEL_COLS + 2, 0, i * (PANEL_COLS + 2));
        wattron(containers[i], COLOR_PAIR(1));
        wbkgd(containers[i], COLOR_PAIR(1));
        panels[i] = derwin(containers[i], PANEL_ROWS, PANEL_COLS, 1, 1);
        box(containers[i], ACS_VLINE, ACS_HLINE);
        wrefresh(containers[i]);
        wrefresh(panels[i]);
        state->panels[i].container = containers[i];
        state->panels[i].panel = panels[i];
    }
    return 0;
}

int render_panel(struct panel *p, int is_focus)
{
    werase(p->panel);

    int begin = p->top;
    int end = ((p->sz < begin + PANEL_ROWS) ? p->sz : begin + PANEL_ROWS);
    int selected = p->selected;

    for (int i = begin; i < end; ++i)
    {
        struct dirent *item = p->namelist[i];
        char *pattern = ((item->d_type == DT_DIR ||
                          item->d_type == DT_LNK)
                             ? "/%s"
                             : "%s");
        int cp = ((is_focus && i == selected) ? 2 : 1);

        wattron(p->panel, COLOR_PAIR(cp));
        mvwprintw(p->panel, i - begin, 0, pattern, item->d_name);
        wattroff(p->panel, COLOR_PAIR(cp));
    }
    wrefresh(p->panel);
    return 0;
}

int render_all(struct state *st)
{
    for (int i = 0; i < NPANELS; ++i)
    {
        render_panel(&st->panels[i], i == st->focus_index);
    }
    return 0;
}
