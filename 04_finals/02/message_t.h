#ifndef MESSAGE_T
#define MESSAGE_T

#include <sys/types.h>

typedef enum
{
    MSG_SEND_TASK,
    MSG_GET_STATUS,
    MSG_AVAIL,
    MSG_BUSY,
} msg_type_t;

typedef struct
{
    msg_type_t msg_type;
    union
    {
        time_t busy_secs;
    } args;
    
} message_t;

#endif