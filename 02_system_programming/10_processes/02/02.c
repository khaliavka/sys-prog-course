#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int process1(void);
int process2(void);
int process3(void);
int process4(void);
int process5(void);

int process1(void)
{
    printf("process1: pid = %d, ppid = %d\n", getpid(), getppid());

    pid_t pid3 = fork();
    if (pid3 == 0)
    {
        process3();
        exit(3);
    }

    pid_t pid4 = fork();
    if (pid4 == 0)
    {
        process4();
        exit(4);
    }

    int status;
    wait(&status);
    printf("status 3|4 = %d\n", WEXITSTATUS(status));
    wait(&status);
    printf("status 3|4 = %d\n", WEXITSTATUS(status));
    return 0;
}

int process2(void)
{
    printf("process2: pid = %d, ppid = %d\n", getpid(), getppid());
    
    pid_t pid5 = fork();
    if (pid5 == 0)
    {
        process5();
        exit(5);
    }

    int status;
    wait(&status);
    printf("status5 = %d\n", WEXITSTATUS(status));
    return 0;
}

int process3(void)
{
    printf("process3: pid = %d, ppid = %d\n", getpid(), getppid());
    return 0;
}

int process4(void)
{
    printf("process4: pid = %d, ppid = %d\n", getpid(), getppid());
    return 0;
}

int process5(void)
{
    printf("process5: pid = %d, ppid = %d\n", getpid(), getppid());
    return 0;
}

int main(void)
{
    printf("process0: pid = %d, ppid = %d\n", getpid(), getppid());

    pid_t pid1 = fork();

    if (pid1 == 0)
    {
        process1();
        exit(1);
    }

    pid_t pid2 = fork();

    if (pid2 == 0)
    {
        process2();
        exit(2);
    }

    int status;
    wait(&status);
    printf("status 1|2 = %d\n", WEXITSTATUS(status));
    wait(&status);
    printf("status 1|2 = %d\n", WEXITSTATUS(status));

    return 0;
}
