#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <mqueue.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pthread.h>
#include <stdatomic.h>

#define HELLOMQ "/hello_mq"
#define MSG_PRIO 0
#define BUF_SZ 1024
#define USERS_MAX_COUNT 20
#define USLEEPTM 100000

struct user_t
{
    char username[1024];
    mqd_t outmsg_mqd;
    mqd_t outusr_mqd;
    mqd_t inmsg_mqd;
    int is_del_marked;
};

struct users_t
{
    struct user_t users[USERS_MAX_COUNT];
    int count;
    pthread_mutex_t mtx;
} gl_users = {.count = 0, .mtx = PTHREAD_MUTEX_INITIALIZER};

_Atomic int cancel_flag = 0;

int add_user(mqd_t hello_mqd)
{
    pthread_mutex_lock(&gl_users.mtx);
    if (gl_users.count == USERS_MAX_COUNT)
    {
        pthread_mutex_unlock(&gl_users.mtx);
        return -1;
    }
    pthread_mutex_unlock(&gl_users.mtx);

    char hello_buf[BUF_SZ];
    if (mq_receive(hello_mqd, hello_buf, sizeof(hello_buf), NULL) == -1)
    {
        if (errno == EAGAIN)
        {
            return -1;
        }
        perror("mq_receive");
        mq_close(hello_mqd);
        exit(EXIT_FAILURE);
    }
    struct mq_attr attr = {
        .mq_flags = 0,
        .mq_maxmsg = 10,
        .mq_msgsize = BUF_SZ,
        .mq_curmsgs = 0};
    char *username = strtok(hello_buf, "|");
    char *outmsg_name = strtok(NULL, "|");
    char *outusr_name = strtok(NULL, "|");
    char *inmsg_name = strtok(NULL, "|");
    mqd_t outmsg_mqd = mq_open(outmsg_name, O_RDWR | O_NONBLOCK, 0600, &attr);
    if (outmsg_mqd == -1)
    {
        perror("mq_open");
        mq_close(hello_mqd);
        exit(EXIT_FAILURE);
    }
    mqd_t outusr_mqd = mq_open(outusr_name, O_RDWR | O_NONBLOCK, 0600, &attr);
    if (outusr_mqd == -1)
    {
        perror("mq_open");
        mq_close(hello_mqd);
        exit(EXIT_FAILURE);
    }
    mqd_t inmsg_mqd = mq_open(inmsg_name, O_RDWR | O_NONBLOCK, 0600, &attr);
    if (inmsg_mqd == -1)
    {
        perror("mq_open");
        mq_close(hello_mqd);
        exit(EXIT_FAILURE);
    }

    pthread_mutex_lock(&gl_users.mtx);
    struct user_t *user = &gl_users.users[gl_users.count];
    strncpy(user->username, username, sizeof(user->username));
    user->outmsg_mqd = outmsg_mqd;
    user->outusr_mqd = outusr_mqd;
    user->inmsg_mqd = inmsg_mqd;
    user->is_del_marked = 0;
    ++(gl_users.count);
    pthread_mutex_unlock(&gl_users.mtx);

    return 0;
}

int delete_users(void)
{
    int ret;
    mqd_t fds_del[3 * USERS_MAX_COUNT];
    int fds_del_count = 0;

    pthread_mutex_lock(&gl_users.mtx);
    int old_count = gl_users.count;
    int i = 0;
    while (i < gl_users.count)
    {
        struct user_t *u = &gl_users.users[i];
        if (u->is_del_marked == 0)
        {
            ++i;
            continue;
        }
        fds_del[fds_del_count++] = u->inmsg_mqd;
        fds_del[fds_del_count++] = u->outusr_mqd;
        fds_del[fds_del_count++] = u->outmsg_mqd;
        if (i < gl_users.count - 1)
        {
            struct user_t *dest = u;
            struct user_t *src = &gl_users.users[gl_users.count - 1];
            strncpy(dest->username, src->username, sizeof(dest->username));
            dest->outmsg_mqd = src->outmsg_mqd;
            dest->outusr_mqd = src->outusr_mqd;
            dest->inmsg_mqd = src->inmsg_mqd;
            dest->is_del_marked = src->is_del_marked;
        }
        --gl_users.count;
    }
    ret = (gl_users.count != old_count ? 0 : -1);
    pthread_mutex_unlock(&gl_users.mtx);

    for (int i = 0; i < fds_del_count; ++i)
    {
        if (mq_close(fds_del[i]) == -1)
        {
            perror("mq_close");
            exit(EXIT_FAILURE);
        }
    }
    return ret;
}

int mark_user_deletion(struct users_t *users, int index)
{
    users->users[index].is_del_marked = 1;
    return 0;
}

void *do_users_thread(void *args)
{
    struct mq_attr attr = {
        .mq_flags = 0,
        .mq_maxmsg = 10,
        .mq_msgsize = BUF_SZ,
        .mq_curmsgs = 0};
        mqd_t hello_mqd = mq_open(HELLOMQ, O_CREAT | O_NONBLOCK | O_RDWR, 0600, &attr);
        if (hello_mqd == -1)
        {
        perror("mq_open");
        exit(EXIT_FAILURE);
    }
    while (atomic_load(&cancel_flag) == 0)
    {
        
        int del_usr = delete_users();
        int add_usr = add_user(hello_mqd);
        if ( del_usr == -1 && add_usr == -1)
        {
            usleep(USLEEPTM);
            continue;
        }
        char users_buf[BUF_SZ] = {0};

        pthread_mutex_lock(&gl_users.mtx);
        for (int i = 0; i < gl_users.count; ++i)
        {
            int len = strlen(users_buf);
            snprintf(users_buf + len, sizeof(users_buf) - len + 1, "%s\n", gl_users.users[i].username);
        }
        for (int i = 0; i < gl_users.count; ++i)
        {
            if (mq_send(gl_users.users[i].outusr_mqd, users_buf, sizeof(users_buf), MSG_PRIO) == -1)
            {
                perror("mq_send");
                exit(EXIT_FAILURE);
            }
        }
        pthread_mutex_unlock(&gl_users.mtx);
    }
    if (mq_close(hello_mqd) == -1)
    {
        perror("mq_close");
    }
    if (mq_unlink(HELLOMQ) == -1)
    {
        perror("mq_unlink");
    }
    return NULL;
}

void *do_messages_thread(void *args)
{
    while (atomic_load(&cancel_flag) == 0)
    {
        char in_msg[BUF_SZ];
        char out_msg[BUF_SZ];

        pthread_mutex_lock(&gl_users.mtx);
        for (int i = 0; i < gl_users.count; ++i)
        {
            if (mq_receive(gl_users.users[i].inmsg_mqd, in_msg, sizeof(in_msg), NULL) == -1)
            {
                if (errno == EAGAIN)
                    continue;
                perror("mq_receive");
                exit(EXIT_FAILURE);
            }
            if (strncmp(in_msg, "exit", 5) == 0)
            {
                snprintf(out_msg, sizeof(out_msg), "%s has left the chat.", (char *)&gl_users.users[i].username);
                mark_user_deletion(&gl_users, i);
            }
            else
            {
                snprintf(out_msg, sizeof(out_msg), "%s: %s", (char *)&gl_users.users[i].username, in_msg);
            }
            for (int i = 0; i < gl_users.count; ++i)
            {
                if (gl_users.users[i].is_del_marked == 1)
                    continue;
                if (mq_send(gl_users.users[i].outmsg_mqd, out_msg, sizeof(out_msg), MSG_PRIO) == -1)
                {
                    perror("mq_send");
                    exit(EXIT_FAILURE);
                }
            }
        }
        pthread_mutex_unlock(&gl_users.mtx);
        usleep(USLEEPTM);
    }
    return NULL;
}

void *do_commands_thread(void *args)
{
    char command[BUF_SZ];
    while (atomic_load(&cancel_flag) == 0)
    {
        printf("Toy chatserver (print exit): ");
        fgets(command, sizeof(command), stdin);
        if (strncmp(command, "exit\n", 6) == 0)
            atomic_store(&cancel_flag, 1);
    }
    return NULL;
}

int main(void)
{
    pthread_t threads[3];
    pthread_create(&threads[0], NULL, do_commands_thread, NULL);
    pthread_create(&threads[1], NULL, do_users_thread, NULL);
    pthread_create(&threads[2], NULL, do_messages_thread, NULL);

    pthread_join(threads[0], NULL);
    pthread_join(threads[1], NULL);
    pthread_join(threads[2], NULL);

    return EXIT_SUCCESS;
}
