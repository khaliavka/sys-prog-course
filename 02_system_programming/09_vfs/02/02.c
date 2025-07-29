#define _DEFAULT_SOURCE
#include <curses.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#define PANEL_HEIGHT 20
#define PANEL_WIDTH 80
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
  int focus_index; // Panel in focus (0 - left, 1 - right)
};

int key_up(struct panel *p)
{
  if (p->selected == 0)
    return 0;

  if (p->selected == p->top)
    --(p->top);

  --(p->selected);
  return 0;
}

int key_down(struct panel *p)
{
  if (p->selected == (p->sz - 1))
    return 0;
  if (p->selected == p->top + PANEL_HEIGHT - 3)
    ++(p->top);

  ++(p->selected);
  return 0;
}

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
    containers[i] = newwin(PANEL_HEIGHT, PANEL_WIDTH, 0, i * PANEL_WIDTH);
    wattron(containers[i], COLOR_PAIR(1));
    wbkgd(containers[i], COLOR_PAIR(1));
    panels[i] = derwin(containers[i], PANEL_HEIGHT - 2, PANEL_WIDTH - 2, 1, 1);
    box(containers[i], ACS_VLINE, ACS_HLINE);
    wrefresh(containers[i]);
    wrefresh(panels[i]);
    state->panels[i].container = containers[i];
    state->panels[i].panel = panels[i];
  }
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
    if (bdt == DT_DIR)
      return alphasort(a, b);
    return -1;

  case DT_LNK:
    if (bdt == DT_DIR)
      return 1;
    if (bdt == DT_LNK)
      return alphasort(a, b);
    return -1;

  case DT_REG:
    if (bdt == DT_DIR)
      return 1;
    if (bdt == DT_LNK)
      return 1;
    if (bdt == DT_REG)
      return alphasort(a, b);
    return -1;

  default:
    break;
  }
  return alphasort(a, b);
}

int render_panel(struct panel *p, int is_focus)
{
  werase(p->panel);

  int begin = p->top;
  int end = ((p->sz < begin + PANEL_HEIGHT) ? p->sz : begin + PANEL_HEIGHT);
  int selected = p->selected;

  for (int i = begin; i < end; ++i)
  {
    struct dirent *item = p->namelist[i];
    char *pattern = ((item->d_type == DT_DIR) ? "/%s" : "%s");
    int cp = ((is_focus && i == selected) ? 2 : 1);

    wattron(p->panel, COLOR_PAIR(cp));
    mvwprintw(p->panel, i - begin, 0, pattern, item->d_name);
    wattroff(p->panel, COLOR_PAIR(cp));
  }
  wrefresh(p->panel);
  return 0;
}

int free_namelist(struct dirent **namelist, int sz)
{
  for (int i = 0; i < sz; ++i)
  {
    free(namelist[i]);
  }
  return 0;
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

int init_state(struct state *st)
{
  st->focus_index = 0;
  for (int i = 0; i < NPANELS; ++i)
  {
    st->panels[i].raw_path = malloc(PATH_MAX);
    st->panels[i].resolved_path = malloc(PATH_MAX);
    st->panels[i].prev_path = malloc(PATH_MAX);

    getcwd(st->panels[i].resolved_path, PATH_MAX);
    strncpy(st->panels[i].prev_path, st->panels[i].resolved_path, PATH_MAX);
    st->panels[i].sz = scandir(st->panels[i].resolved_path, &st->panels[i].namelist, filter_self, category_alphasort);
    st->panels[i].top = 0;
    st->panels[i].selected = 0;
  }
  return 0;
}

void cleanup(struct state *st)
{
  for (int i = 0; i < NPANELS; ++i)
  {
    delwin(st->panels[i].panel);
    delwin(st->panels[i].container);

    free(st->panels[i].prev_path);
    free(st->panels[i].raw_path);
    free(st->panels[i].resolved_path);
    free_namelist(st->panels[i].namelist, st->panels[i].sz);
  }
  endwin();
}

int render_all(struct state *st)
{
  for (int i = 0; i < NPANELS; ++i)
  {
    render_panel(&st->panels[i], i == st->focus_index);
  }
  return 0;
}

void switch_focus(struct state *st)
{
  st->focus_index = (st->focus_index + 1) % 2;
}

int main(void)
{

  struct state st;
  init_gui(&st, sig_winch);
  init_state(&st);
  render_all(&st);
  int ch;
  while ((ch = getch()) != '\e' && ch != 'q')
  {
    switch (ch)
    {
    case KEY_UP:
      key_up(&st.panels[st.focus_index]);
      break;

    case KEY_DOWN:
      key_down(&st.panels[st.focus_index]);
      break;

    case '\t':
      switch_focus(&st);
      break;

    case '\n':
      enter_dir(&st.panels[st.focus_index]);
      break;
      
    default:
      break;
    }
    render_all(&st);
  }

  cleanup(&st);

  return EXIT_SUCCESS;
}