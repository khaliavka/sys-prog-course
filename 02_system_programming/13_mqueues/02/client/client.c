#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <mqueue.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <unistd.h>

#include "client.h"
#include "gui.h"

int main(void)
{
    struct state st;
    init_gui(&st);
    read_message(st.enterw.wnd);
    cleanup(&st);
    return EXIT_SUCCESS;
}
