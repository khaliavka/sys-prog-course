#ifndef INPUT_H
#define INPUT_H

#include "commands.h"

#define INPUTBUF_SIZE 100

int read_line(char *line, int sz);
int parse_command(char *str, int sz, command_args_t *cmd_args);

#endif