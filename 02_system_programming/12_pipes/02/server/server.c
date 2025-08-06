#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#define FIFO_PATH "/tmp/hi_pipe"

int main(void)
{
    int fd;
    const char *msg = "Hi!\n";
    if (mkfifo(FIFO_PATH, 0666) == -1)
    {
        perror("mkfifo");
        exit(EXIT_FAILURE);
    }
    fd = open(FIFO_PATH, O_WRONLY);
    if (fd == -1)
    {
        perror("open");
        exit(EXIT_FAILURE);
    }
    if (write(fd, msg, strlen(msg)) == -1)
    {
        perror("write");
        close(fd);
        exit(EXIT_FAILURE);
    }
    printf("Sent message: %s", msg);
    close(fd);
    if (unlink(FIFO_PATH) == -1)
    {
        perror("unlink");
        exit(EXIT_FAILURE);
    }
    return EXIT_SUCCESS;
}
