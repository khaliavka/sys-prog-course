#ifndef CONNECT_H
#define CONNECT_H

#include <mqueue.h>

#include "gui.h"

#define MSG_BUFSZ 1024
#define MSGS_COUNT 20
#define USERNM_BUFSZ 21
#define USERNM_COUNT 20
#define MQNAMESZ 256
#define HELLOSZ (MQNAMESZ * 4)

struct msgs_buf_t;

char (*get_buf(struct msgs_buf_t *buf))[MSG_BUFSZ];
int get_insert_idx(struct msgs_buf_t *buf);
int receive_message(mqd_t mqd, struct msgs_buf_t *msgs_buf);
int receive_usernames(mqd_t mqd, char *usernames, int sz);
int send_input(mqd_t mqd, char *input_msg, int sz);
int print_msgs_buf(WINDOW *wnd, struct msgs_buf_t *messages);
void *do_msgs_thread(void *args);
void *do_usernames_thread(void *args);
void *do_input_msg_thread(void *args);

#endif