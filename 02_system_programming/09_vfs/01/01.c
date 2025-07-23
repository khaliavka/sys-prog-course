#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

int write_str(const char *filename, char *buf, int count)
{
    int fd = open(filename, O_CREAT | O_WRONLY | O_TRUNC, 0664);
    if (fd == -1)
        return -1;

    int wr = write(fd, buf, count);
    close(fd);
    return wr;
}

int read_str_reverse(const char *filename, char *buf, int count)
{
    int fd = open(filename, O_RDONLY, 0664);
    if (fd == -1)
        return -1;

    // Comment: For educational purposes only.
    // One syscall every iteration. (Slow!)

    for (int i = 0; i < count; ++i)
    {
        if (pread(fd, buf + i, 1, count - i - 1) == -1)
        {
            close(fd);
            perror("pread");
            return -1;
        }
    }
    close(fd);
    return count;
}

void print(const char *buf, int count)
{
    write(STDOUT_FILENO, buf, count);

    const char *nl = "\n";
    write(STDOUT_FILENO, nl, 1);
}

int main(void)
{
    const char *filename = "output.txt";
    char buf[] = "String from file";

    int wr = write_str(filename, buf, sizeof(buf) - 1);
    int rd = read_str_reverse(filename, buf, wr);
    print(buf, rd);

    return 0;
}