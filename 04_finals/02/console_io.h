#ifndef CONSOLE_IO_H
#define CONSOLE_IO_H

#include "commands.h"
#include "message_t.h"

#define INPUTBUF_SIZE 100

int read_line(char *line, int sz);
int parse_command(char *str, int sz, command_args_t *cmd_args);
int read_command(command_args_t *cmd_args);
int print_message(message_t *msg);

#endif