#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

int my_file_puts(int fd, const char *s);
int countChars(const char *s);

int main(int argc, char **argv)
{
    int fd = open("test.txt",O_CREAT|O_RDWR);
    my_file_puts(fd, "whooaaaa");
    close(fd);
    return 0;
}

int countChars(const char *s)
{
    int i = 0;
    while (s)
    {
        if (s[i] != '\0')
        i++;
        else break;
    } 
    return i;
}
int my_file_puts(int fd, const char *s)
{
    int inputNumber = countChars(s);
    if (fd)
    {
        write(fd, s, inputNumber);
    }
    return inputNumber;
}

