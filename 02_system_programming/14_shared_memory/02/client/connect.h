#ifndef CONNECT_H
#define CONNECT_H


#include "gui.h"
#include "shmbuf_t.h"

#define MSG_BUFSZ 1024
#define MSGS_COUNT 20
#define USERNM_BUFSZ 21
#define USERNM_COUNT 20
#define MQNAMESZ 256
#define HELLOSZ (MQNAMESZ * 4)

struct shmbuf_t *create_shmqueue(const char *name);
int delete_shmqueue(struct shmbuf_t *shmp, const char *name);
struct shmbuf_t *open_shmqueue(const char *name);
int close_shmqueue(struct shmbuf_t *shmp, const char *name);

#endif