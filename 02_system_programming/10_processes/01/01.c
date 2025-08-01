#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main(void)
{
    pid_t pid = fork();
    if (pid == 0)
    {
        // child
        int exit_status = 15;
        printf("The child process: pid = %d, ppid = %d, exit status = %d\n", getpid(), getppid(), exit_status);
        exit(exit_status);
    }
    // parent
    printf("The parent process: pid = %d, ppid = %d, the child's pid = %d\n",
           getpid(), getppid(), pid);

    int child_status;
    wait(&child_status);
    printf("The parent process: The child's exit status = %d\n", WEXITSTATUS(child_status));
    return 0;
}
