#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <unistd.h>

#define MSGSND_TYPE 2
#define MSGRCV_TYPE 1
struct message
{
    long mtype;
    char mtext[256];
};

key_t get_key(void)
{
    key_t key = ftok(".", 'a');
    if (key == -1)
    {
        perror("ftok");
        exit(EXIT_FAILURE);
    }
    return key;
}

int main(void)
{
    int msqid = msgget(get_key(), IPC_CREAT | 0600);
    if (msqid == -1)
    {
        perror("msgget");
        exit(EXIT_FAILURE);
    }
    struct message msg_rcv;
    if (msgrcv(msqid, &msg_rcv, sizeof(msg_rcv.mtext), MSGRCV_TYPE, 0) == -1)
    {
        perror("msgrcv");
        exit(EXIT_FAILURE);
    }
    printf("%s\n", msg_rcv.mtext);
    
    struct message msg_send = {.mtype = MSGSND_TYPE, .mtext = "Hello!"};
    if (msgsnd(msqid, &msg_send, sizeof(msg_send.mtext), 0) == -1)
    {
        perror("msgsnd");
        exit(EXIT_FAILURE);
    }
    
    return EXIT_SUCCESS;
}
