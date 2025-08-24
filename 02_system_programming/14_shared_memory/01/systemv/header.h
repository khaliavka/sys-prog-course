#ifndef HEADER_H
#define HEADER_H

#define err_exit(msg)    do { perror(msg); exit(EXIT_FAILURE); \
                                   } while (0)

#define BUF_SIZE 10
#define NSEMS 2
#define SEMNUMSERVER 0
#define SEMNUMCLIENT 1

#endif