#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/random.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <unistd.h>
#include <pthread.h>
#include <stdatomic.h>

#include "client.h"
#include "exitmacro.h"
#include "gui.h"
#include "connect.h"
#include "shmbuf_t.h"
#include "msgsbuf_t.h"

#define HELLOSHM "/hello_shm"
#define USLEEPTM 100000
#define SHORT_PROMPT "Message"

extern pthread_mutex_t ncurses_mtx;
_Atomic int cancel_flag = 0;


void *do_msgs_thread(void *args)
{
    struct msgsbuf_t messages = {.insert_idx = 0};

    WINDOW *wnd = ((struct args_t *)args)->wnd;
    struct shmbuf_t *shmp = ((struct args_t *)args)->shmp;
    while (atomic_load(&cancel_flag) == 0)
    {
        if (receive_message(shmp, &messages) == -1)
        {
            usleep(USLEEPTM);
            continue;
        }
        pthread_mutex_lock(&ncurses_mtx);
        print_msgsbuf(wnd, &messages);
        pthread_mutex_unlock(&ncurses_mtx);
    }
    return NULL;
}

void *do_usernames_thread(void *args)
{
    char usernames[1024];
    WINDOW *wnd = ((struct args_t *)args)->wnd;
    struct shmbuf_t *shmp = ((struct args_t *)args)->shmp;
    while (atomic_load(&cancel_flag) == 0)
    {
        if (receive_usernames(shmp, usernames, sizeof(usernames)) == -1)
        {
            usleep(USLEEPTM);
            continue;
        }
        pthread_mutex_lock(&ncurses_mtx);
        print_buffer(wnd, usernames);
        pthread_mutex_unlock(&ncurses_mtx);
    }
    return NULL;
}

void *do_input_msg_thread(void *args)
{
    char input_msg[MSG_BUFSZ];
    WINDOW *wnd = ((struct args_t *)args)->wnd;
    struct shmbuf_t *shmp = ((struct args_t *)args)->shmp;
    while (atomic_load(&cancel_flag) == 0)
    {
        read_input(wnd, input_msg, sizeof(input_msg), SHORT_PROMPT);
        if (strncmp(input_msg, "exit", 5) == 0)
        {
            atomic_store(&cancel_flag, 1);
        }
        if (send_message(shmp, input_msg, sizeof(input_msg)) == -1)
            usleep(USLEEPTM);
    }
    return NULL;
}

int main(void)
{
    struct windows_t windows;
    const char *patterns[3] = {
        "/%s_msg_in_%lu",
        "/%s_username_in_%lu",
        "/%s_msg_out_%lu",
    };
    char my_username[USERNM_BUFSZ];
    char names[3][MQNAMESZ];
    struct shmbuf_t *shmbufs[3];

    char hello_buf[HELLOSZ];
    unsigned long shmid;

    init_gui();
    draw_gui(&windows);
    get_box_input((MSGWND_ROWS + INPUTWND_ROWS + 2) / 2,
                  (INPUTWND_COLS - USERNAME_COLS - 19) / 2,
                  my_username, USERNM_BUFSZ, "Enter your username");
    refresh_windows(&windows);

    getrandom(&shmid, sizeof(shmid), 0);

    for (int i = 0; i < 3; ++i)
    {
        snprintf((char *)&names[i], sizeof(names[0]), patterns[i], my_username, shmid);
        shmbufs[i] = create_shmqueue((const char *)&names[i]);
    }

    struct shmbuf_t *hello_shmp = open_shmqueue(HELLOSHM);
    snprintf(hello_buf, sizeof(hello_buf), "%s|%s|%s|%s", my_username, names[0], names[1], names[2]);
    if (shmbuf_send(hello_shmp, hello_buf, sizeof(hello_buf)) == -1)
        err_exit("shmbuf_send");

    pthread_t msgs_thread;
    pthread_t usrnms_thread;
    pthread_t inputmsg_thread;
    struct args_t msgs_args = {.wnd = windows.messages.wnd, .shmp = shmbufs[0]};
    pthread_create(&msgs_thread, NULL, do_msgs_thread, &msgs_args);
    struct args_t usrnms_args = {.wnd = windows.users.wnd, .shmp = shmbufs[1]};
    pthread_create(&usrnms_thread, NULL, do_usernames_thread, &usrnms_args);
    struct args_t inputmsg_args = {.wnd = windows.input.wnd, .shmp = shmbufs[2]};
    pthread_create(&inputmsg_thread, NULL, do_input_msg_thread, &inputmsg_args);
    pthread_join(msgs_thread, NULL);
    pthread_join(usrnms_thread, NULL);
    pthread_join(inputmsg_thread, NULL);

    close_shmqueue(hello_shmp);
    for (int i = 0; i < 3; ++i)
    {
        delete_shmqueue(shmbufs[i], (char *)&names[i]);
    }
    cleanup_gui(&windows);
    return EXIT_SUCCESS;
}
