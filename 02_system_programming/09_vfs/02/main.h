#ifndef MAIN_H
#define MAIN_H

// #define _DEFAULT_SOURCE
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

#include "nav.h"

int init_state(struct state *st);
int free_namelist(struct dirent **namelist, int sz);
void cleanup(struct state *st);

#endif