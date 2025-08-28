#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

#include "exitmacro.h"

int main(void)
{
    
    sigset_t mask, old_mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGUSR1);
    printf("PID = %d, Blocking SIGUSR1...\n", getpid());
    if (sigprocmask(SIG_BLOCK, &mask, &old_mask) == -1)
        err_exit("sigprocmask");
    while (1)
    {
        printf("Waiting for pending SIGUSR1...\n");
        int sig;
        if (sigwait(&mask, &sig) > 0)
            err_exit("sigwait");
        printf("Pending SIGUSR1 detected.\n");
    }

    return EXIT_SUCCESS;
}
