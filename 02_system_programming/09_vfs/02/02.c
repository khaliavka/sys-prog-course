#include <termios.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <curses.h>

void sig_winch(int signo)
{
  (void) signo;
  struct winsize size;
  ioctl(fileno(stdout), TIOCGWINSZ, (char *)&size);
  resizeterm(size.ws_row, size.ws_col);
}

int main(void)
{
  WINDOW *left_container, *right_container;
  WINDOW *left_panel, *right_panel;
  initscr();
  signal(SIGWINCH, sig_winch);
  curs_set(FALSE);
  start_color();

  cbreak();
  noecho();
  refresh();

  init_pair(1, COLOR_WHITE, COLOR_BLUE);
  init_pair(2, COLOR_BLACK, COLOR_CYAN);
  int height = 20;
  int width = 80;
  left_container = newwin(height, width, 0, 0);
  right_container = newwin(height, width, 0, width);
  wattron(left_container, COLOR_PAIR(1));
  wattron(right_container, COLOR_PAIR(1));
  wbkgd(left_container, COLOR_PAIR(1));
  wbkgd(right_container, COLOR_PAIR(1));

  left_panel = derwin(left_container, height - 2, width - 2, 1, 1);
  right_panel = derwin(right_container, height - 2, width - 2, 1, 1);
  box(left_container, '|', '-');
  box(right_container, '|', '-');
  wprintw(left_panel, "Left!\n");

  wprintw(right_panel, "Right!\n");
  wrefresh(left_container);
  wrefresh(right_container);
  refresh();
  getch();
  
  char *list[] = {
    "Item 1",
    "Item 2",
    "Item 3",
    "Item 4",
    "Item 5"
  };
  
  werase(right_panel);
  int selected = 3;
  for (int i = 0; i < 5; ++i)
  {
    if (i == selected)
      wattron(right_panel, COLOR_PAIR(2));
    else
      wattron(right_panel, COLOR_PAIR(1));
    mvwprintw(right_panel, i, 1, "%s", list[i]);
    wattroff(right_panel, COLOR_PAIR(1));
    wattroff(right_panel, COLOR_PAIR(2));
  }
  wrefresh(right_panel);
  getch();

  delwin(left_panel);
  delwin(right_panel);
  delwin(left_container);
  delwin(right_container);
  endwin();
  return EXIT_SUCCESS;
}