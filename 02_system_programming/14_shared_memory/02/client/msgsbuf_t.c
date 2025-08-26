#include "msgsbuf_t.h"
#include "shmbuf_t.h"

int get_insert_idx(struct msgsbuf_t *buf)
{
    return buf->insert_idx;
}

char (*get_buf(struct msgsbuf_t *buf))[MSGBUF_SIZE]
{
    return buf->buf;
}

int receive_message(struct shmbuf_t *shmp, struct msgsbuf_t *msgs_buf)
{
    int idx = msgs_buf->insert_idx % MSGS_COUNT;

    if (shmbuf_receive(shmp, (char *)&msgs_buf->buf[idx], sizeof(msgs_buf->buf[0])) == -1)
        return -1;
    ++(msgs_buf->insert_idx);
    return 0;
}

int receive_usernames(struct shmbuf_t *shmp, char *usernames, int sz)
{
    if (shmbuf_receive(shmp, usernames, sz) == -1)
        return -1;
    return 0;
}

int send_message(struct shmbuf_t *shmp, char *input_msg, int sz)
{
    if (shmbuf_send(shmp, input_msg, sz) == -1)
        return -1;
    return 0;
}