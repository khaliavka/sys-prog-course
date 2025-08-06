#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <wordexp.h>

#define LINE_MAX 1024

int main(void)
{
    char line[LINE_MAX];
    wordexp_t we;
    while (1)
    {
        printf("Enter a command: ");
        if (fgets(line, sizeof(line), stdin) == NULL)
        {
            perror("fgets");
            exit(1);
        }

        line[strcspn(line, "\n")] = '\0';

        if (wordexp(line, &we, 0) != 0)
        {
            perror("wordexp");
            exit(1);
        }
        if (strncmp(we.we_wordv[0], "exit", 5) == 0)
        {
            wordfree(&we);
            return EXIT_SUCCESS;
        }

        pid_t pid = fork();
        if (pid == 0)
        {
            execvp(we.we_wordv[0], we.we_wordv);
            perror("execvp");
            exit(1);
        }
        wordfree(&we);
        wait(NULL);
    }

    return EXIT_FAILURE;
}
