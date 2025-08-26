#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pthread.h>
#include <stdatomic.h>

#include "shmbuf_t.h"
#include "connect.h"

#define HELLOSHM "/hello_shm"
#define BUF_SZ 1024
#define USERS_MAX_COUNT 20
#define USLEEPTM 100000

struct user_t
{
    struct shmbuf_t *outmsg_shmp;
    struct shmbuf_t *outusr_shmp;
    struct shmbuf_t *inmsg_shmp;
    int is_del_marked;
    char username[32];
};

struct users_t
{
    struct user_t users[USERS_MAX_COUNT];
    int count;
    pthread_mutex_t mtx;
} gl_users = {.count = 0, .mtx = PTHREAD_MUTEX_INITIALIZER};

_Atomic int cancel_flag = 0;

int add_user(struct shmbuf_t *hello_shm)
{
    pthread_mutex_lock(&gl_users.mtx);
    if (gl_users.count == USERS_MAX_COUNT)
    {
        pthread_mutex_unlock(&gl_users.mtx);
        return -1;
    }
    pthread_mutex_unlock(&gl_users.mtx);

    char hello_buf[BUF_SZ];
    if (shmbuf_receive(hello_shm, hello_buf, sizeof(hello_buf)) == -1)
        return -1;
    
    char *username = strtok(hello_buf, "|");
    char *outmsg_name = strtok(NULL, "|");
    char *outusr_name = strtok(NULL, "|");
    char *inmsg_name = strtok(NULL, "|");
    struct shmbuf_t *outmsg_shmp = open_shmqueue(outmsg_name);
    struct shmbuf_t *outusr_shmp = open_shmqueue(outusr_name);
    struct shmbuf_t *inmsg_shmp = open_shmqueue(inmsg_name);

    pthread_mutex_lock(&gl_users.mtx);
    struct user_t *user = &gl_users.users[gl_users.count];
    strncpy(user->username, username, sizeof(user->username));
    user->outmsg_shmp = outmsg_shmp;
    user->outusr_shmp = outusr_shmp;
    user->inmsg_shmp = inmsg_shmp;
    user->is_del_marked = 0;
    ++(gl_users.count);
    pthread_mutex_unlock(&gl_users.mtx);

    return 0;
}

int delete_users(void)
{
    int ret;
    struct shmbuf_t *shmps_del[3 * USERS_MAX_COUNT];
    int shmps_del_count = 0;

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
        shmps_del[shmps_del_count++] = u->inmsg_shmp;
        shmps_del[shmps_del_count++] = u->outusr_shmp;
        shmps_del[shmps_del_count++] = u->outmsg_shmp;
        if (i < gl_users.count - 1)
        {
            struct user_t *dest = u;
            struct user_t *src = &gl_users.users[gl_users.count - 1];
            strncpy(dest->username, src->username, sizeof(dest->username));
            dest->outmsg_shmp = src->outmsg_shmp;
            dest->outusr_shmp = src->outusr_shmp;
            dest->inmsg_shmp = src->inmsg_shmp;
            dest->is_del_marked = src->is_del_marked;
        }
        --gl_users.count;
    }
    ret = (gl_users.count != old_count ? 0 : -1);
    pthread_mutex_unlock(&gl_users.mtx);

    for (int i = 0; i < shmps_del_count; ++i)
        close_shmqueue(shmps_del[i]);
    return ret;
}

int mark_user_deletion(struct users_t *users, int index)
{
    users->users[index].is_del_marked = 1;
    return 0;
}

void *do_users_thread(void *args)
{
    struct shmbuf_t *hello_shmp = create_shmqueue(HELLOSHM);
    while (atomic_load(&cancel_flag) == 0)
    {
        
        int del_usr = delete_users();
        int add_usr = add_user(hello_shmp);
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
            shmbuf_send(gl_users.users[i].outusr_shmp, users_buf, sizeof(users_buf));
        pthread_mutex_unlock(&gl_users.mtx);
    }
    delete_shmqueue(hello_shmp, HELLOSHM);
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
            if (shmbuf_receive(gl_users.users[i].inmsg_shmp, in_msg, sizeof(in_msg)) == -1)
                continue;
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
                if (shmbuf_send(gl_users.users[i].outmsg_shmp, out_msg, sizeof(out_msg)) == -1)
                {
                    // decide what to do if the client is full
                    perror("shmbuf_send");
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
