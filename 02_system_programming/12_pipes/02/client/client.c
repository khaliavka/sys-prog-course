#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define FIFO_PATH "/tmp/hi_pipe"

int main(void)
{
    int fd;
    char buf[100];
    fd = open(FIFO_PATH, O_RDONLY);
    if (fd == -1)
    {
        perror("open");
        exit(EXIT_FAILURE);
    }
    read(fd, buf, sizeof(buf));
    printf("Received message: %s", buf);
    close(fd);
    return EXIT_SUCCESS;
}
