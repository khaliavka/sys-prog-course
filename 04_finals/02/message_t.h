#ifndef MESSAGE_T
#define MESSAGE_T

#include <sys/types.h>

typedef enum
{
    MSG_SEND_TASK,
    MSG_GET_STATUS,
    MSG_DEL,
    MSG_AVAIL,
    MSG_BUSY
} msg_type_t;

typedef struct
{
    time_t busy_secs;
} msg_send_task_t;

typedef struct {} msg_get_status_t;
typedef struct {} msg_del_t;

typedef struct {
    pid_t pid;
} msg_avail_t;

typedef struct
{
    pid_t pid;
    time_t busy_secs;
} msg_busy_t;

typedef struct
{
    msg_type_t msg_type;
    union
    {
        msg_send_task_t send_task;
        msg_get_status_t get_status;
        msg_del_t del;
        msg_avail_t avail;
        msg_busy_t busy;
    } args;
    
} message_t;

#endif