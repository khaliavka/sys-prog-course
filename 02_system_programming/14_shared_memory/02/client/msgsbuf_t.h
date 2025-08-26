#ifndef MSGSBUF_T_H
#define MSGSBUF_T_H

#include "shmbuf_t.h"

#define MSGS_COUNT 20
#define MSGBUF_SIZE 1024

struct msgsbuf_t
{
    char buf[MSGS_COUNT][MSGBUF_SIZE];
    int insert_idx;
};

int get_insert_idx(struct msgsbuf_t *buf);
char (*get_buf(struct msgsbuf_t *buf))[MSGBUF_SIZE];
int receive_message(struct shmbuf_t *shmp, struct msgsbuf_t *msgs_buf);
int receive_usernames(struct shmbuf_t *shmp, char *usernames, int sz);
int send_message(struct shmbuf_t *shmp, char *input_msg, int sz);

#endif