#ifndef EXITMACRO_H
#define EXITMACRO_H

#include <stdio.h>
#include <stdlib.h>

#define err_exit(msg)    do { perror(msg); exit(EXIT_FAILURE); \
                                   } while (0)

#endif