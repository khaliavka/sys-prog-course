#ifndef NAV_H
#define NAV_H

#include <curses.h>

#define PANEL_ROWS 20
#define PANEL_COLS 80
#define NPANELS 2

struct panel
{
    WINDOW *container;
    WINDOW *panel;

    char *raw_path;
    char *resolved_path;
    char *prev_path;

    struct dirent **namelist;
    int top;
    int selected;
    int sz;
};

struct state
{
    struct panel panels[NPANELS];
    int focus_index;
};

int up_one_line(struct panel *p);

int down_one_line(struct panel *p);

int page_up(struct panel *p);

int page_down(struct panel *p);

int go_up(struct panel *p);

int go_down(struct panel *p);

int filter_self(const struct dirent *d);

int category_alphasort(const struct dirent **a, const struct dirent **b);

void swap_path(char **a, char **b);

int enter_dir(struct panel *p);

void switch_focus(struct state *st);

#endif