#include "nav.h"

#include <limits.h>
#include <dirent.h>

#include "main.h"

int up_one_line(struct panel *p)
{
    if (p->selected == 0)
        return 0;

    if (p->selected == p->top)
        --(p->top);

    --(p->selected);
    return 0;
}

int down_one_line(struct panel *p)
{
    if (p->selected == (p->sz - 1))
        return 0;

    if (p->selected == p->top + PANEL_ROWS - 1)
        ++(p->top);

    ++(p->selected);
    return 0;
}

int page_up(struct panel *p)
{
    p->top = ((p->top < PANEL_ROWS) ? 0 : p->top - PANEL_ROWS);
    p->selected = p->top;
    return 0;
}

int page_down(struct panel *p)
{
    int p_top = p->top;
    int size = p->sz;
    if (size < PANEL_ROWS)
    {
        p->selected = size - 1;
        return 0;
    }

    p->top = ((size - p_top < 2 * PANEL_ROWS) ? size - PANEL_ROWS : p_top + PANEL_ROWS);
    p->selected = p->top + PANEL_ROWS - 1;
    return 0;
}

int go_up(struct panel *p)
{
    p->top = 0;
    p->selected = 0;
    return 0;
}

int go_down(struct panel *p)
{
    p->top = ((p->sz < PANEL_ROWS) ? 0 : p->sz - PANEL_ROWS);
    p->selected = p->sz - 1;
    return 0;
}

int filter_self(const struct dirent *d)
{
    return strncmp(d->d_name, ".", 2) != 0;
}

int category_alphasort(const struct dirent **a, const struct dirent **b)
{
    unsigned char adt = (*a)->d_type;
    unsigned char bdt = (*b)->d_type;
    switch (adt)
    {
    case DT_DIR:
    case DT_LNK:
        switch (bdt)
        {
        case DT_DIR:
        case DT_LNK:
            return alphasort(a, b);
        default:
            return -1;
        }

    case DT_REG:
        switch (bdt)
        {
        case DT_DIR:
        case DT_LNK:
            return 1;
        case DT_REG:
            return alphasort(a, b);
        default:
            return -1;
        }

    case DT_CHR:
        switch (bdt)
        {
        case DT_DIR:
        case DT_LNK:
        case DT_REG:
            return 1;
        case DT_CHR:
            return alphasort(a, b);
        default:
            return -1;
        }
    case DT_BLK:
        switch (bdt)
        {
        case DT_DIR:
        case DT_LNK:
        case DT_REG:
        case DT_CHR:
            return 1;
        case DT_BLK:
            return alphasort(a, b);
        default:
            return -1;
        }
    default:
        break;
    }
    return alphasort(a, b);
}

void swap_path(char **a, char **b)
{
    char *t = *a;
    *a = *b;
    *b = t;
}

int enter_dir(struct panel *p)
{
    unsigned char d_type = p->namelist[p->selected]->d_type;
    if (d_type != DT_DIR && d_type != DT_LNK)
        return 0;

    const char *d_name = p->namelist[p->selected]->d_name;
    snprintf(p->resolved_path + strlen(p->resolved_path), NAME_MAX + 2, "/%s", d_name);
    realpath(p->resolved_path, p->raw_path);
    swap_path(&p->resolved_path, &p->raw_path);
    struct dirent **nm;
    int size = scandir(p->resolved_path, &nm, filter_self, category_alphasort);
    if (size == -1)
    {
        strncpy(p->resolved_path, p->prev_path, PATH_MAX);
        return 1;
    }

    strncpy(p->prev_path, p->resolved_path, PATH_MAX);
    free_namelist(p->namelist, p->sz);
    p->namelist = nm;
    p->sz = size;
    p->top = 0;
    p->selected = 0;
    return 0;
}

void switch_focus(struct state *st)
{
    st->focus_index = (st->focus_index + 1) % 2;
}
