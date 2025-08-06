#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main(void)
{
    int pipefd[2];
    const char msg[] = "Hi!\n";
    int msg_len = strlen(msg);
    if (pipe(pipefd) == -1)
    {
        perror("pipe");
        exit(EXIT_FAILURE);
    }
    pid_t cpid = fork();
    if (cpid == -1)
    {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    if (cpid == 0)
    {
        close(pipefd[1]); // Close write end
        char *buf = malloc(msg_len + 1);
        buf[msg_len] = '\0';
        while(read(pipefd[0], buf, msg_len) > 0)
            write(STDOUT_FILENO, buf, msg_len);
        free(buf);
        close(pipefd[0]);
        exit(EXIT_SUCCESS);
    }
    close(pipefd[0]); // Close read end
    write(pipefd[1], msg, msg_len);
    close(pipefd[1]);
    wait(NULL);
    return EXIT_SUCCESS;
}
