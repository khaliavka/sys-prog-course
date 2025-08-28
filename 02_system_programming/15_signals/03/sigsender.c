#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

#include "exitmacro.h"

int main(void)
{
    printf("Enter a pid: ");
    char inbuf[100];
    if (fgets(inbuf, sizeof(inbuf), stdin) == NULL)
        err_exit("fgets");
    pid_t pid;
    sscanf(inbuf, "%d", &pid);
    printf("Sending SIGUSR1 to %d...\n", pid);
    while (1)
    {
        sleep(1);
        kill(pid, SIGUSR1);
    }

    return EXIT_SUCCESS;
}
