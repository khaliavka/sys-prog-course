#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

#include "exitmacro.h"

void sigusr1_handler(int sig)
{
    (void)sig;
    write(STDOUT_FILENO, "Caught SIGUSR1.\n", 17);
}

int main(void)
{
    struct sigaction sa = {.sa_handler = sigusr1_handler, .sa_flags = 0};
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIGUSR1, &sa, NULL) == -1)
        err_exit("sigaction");

    printf("PID = %d, waiting for SIGUSR1 signal...\n", getpid());
    while (1)
        sleep(1);

    return EXIT_SUCCESS;
}
