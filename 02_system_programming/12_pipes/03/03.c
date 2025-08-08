#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <wordexp.h>

#define LINE_MAX 1024

struct commands
{
    char **cmds;
    int count;
};

int count_commands(const char *str)
{
    if (*str == '\0')
        return 0;

    int ret = 0;
    while (*str)
    {
        if (*str == '|')
            ++ret;
        ++str;
    }
    return ret + 1;
}

int init_commands(struct commands *commands, char *line)
{
    int count = count_commands(line);
    commands->cmds = malloc(count * sizeof(char *));
    commands->count = count;
    *commands->cmds = strtok(line, "|");
    for (int i = 1; i < count; ++i)
    {
        commands->cmds[i] = strtok(NULL, "|");
    }
    return 0;
}

int cleanup_commands(struct commands *commands)
{
    free(commands->cmds);
    commands->count = 0;
    return 0;
}

int run_commands(char *line)
{
    struct commands commands;
    init_commands(&commands, line);

    int left_pipe[2];
    int right_pipe[2];
    for (int i = 0; i < commands.count; ++i)
    {
        wordexp_t we;
        if (wordexp(commands.cmds[i], &we, 0) != 0)
        {
            perror("wordexp");
            break;
        }

        if (i > 1)
        {
            close(left_pipe[0]);
            close(left_pipe[1]);
        }
        left_pipe[0] = right_pipe[0];
        left_pipe[1] = right_pipe[1];
        if (i < commands.count - 1)
        {
            pipe(right_pipe);
        }

        pid_t cpid = fork();
        if (cpid == -1)
        {
            perror("fork");
            wordfree(&we);
            exit(EXIT_FAILURE);
        }

        if (cpid == 0)
        {
            if (i > 0)
            {
                close(left_pipe[1]); // Close write end
                dup2(left_pipe[0], STDIN_FILENO);
                close(left_pipe[0]); // Close original read end
            }
            if (i < commands.count - 1)
            {
                close(right_pipe[0]); // Close read end
                dup2(right_pipe[1], STDOUT_FILENO);
                close(right_pipe[1]); // Close original write end
            }

            execvp(we.we_wordv[0], we.we_wordv);
            perror("execvp");
            wordfree(&we);
            exit(EXIT_FAILURE);
        }

        wordfree(&we);
    }
    if (commands.count > 1)
    {
        close(left_pipe[0]);
        close(left_pipe[1]);
    }

    for (int i = 0; i < commands.count; ++i)
    {
        wait(NULL);
    }
    cleanup_commands(&commands);
    return 0;
}

int read_line(char *line, int sz)
{
    printf("toyshell: ");
    if (fgets(line, sz, stdin) == NULL)
    {
        perror("fgets");
        exit(EXIT_FAILURE);
    }
    line[strcspn(line, "\n")] = '\0';
    return 0;
}

int main(void)
{
    char line[LINE_MAX];
    while (1)
    {
        read_line(line, sizeof(line));

        if (strncmp(line, "exit", 5) == 0)
            return EXIT_SUCCESS;

        run_commands(line);
    }
    return EXIT_FAILURE;
}
