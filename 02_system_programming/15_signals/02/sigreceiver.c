#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

#include "exitmacro.h"

int main(void)
{
    
    sigset_t mask, old_mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGINT);
    printf("PID = %d, Blocking SIGINT...\n", getpid());
    if (sigprocmask(SIG_BLOCK, &mask, &old_mask) == -1)
        err_exit("sigprocmask");
    while (1)
        sleep(1);

    return EXIT_SUCCESS;
}
