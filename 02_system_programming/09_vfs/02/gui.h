#ifndef GUI_H
#define GUI_H

#include "nav.h"

void sig_winch(int signo);
int init_gui(struct state *state, void (*handler)(int));
int render_panel(struct panel *p, int is_focus);
int render_all(struct state *st);

#endif