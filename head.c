#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

int countChars(const char *s);
int my_file_puts(int fd,const char *s);

int atooi(const char *s);
char buf[1];  
int lineMax, lineNo, fd;
int main(int argc, char **argv)
{
    lineNo = 0;
    if (argc == 1)
    {
        lineMax = 10;
        fd = 0;
    }
    else if (argc == 2)
    {
        lineMax = 10;
        fd = open(argv[1],O_RDONLY);
        if (fd < 0) {
           write(fileno(stderr),strerror(errno),countChars(strerror(errno)));
           return 1;
        }

    }
    else if (argc == 3)
    {
         if (argv[1][1] != 'n' || argv[1][0] != '-' || argv[1][2] != '\0')
            {
                write(fileno(stderr),"did you mean -n?",20);
                return 1;
            }
            lineMax = atooi(argv[2]);
            fd = 0;
    }
    else if (argc == 4)
    {
        if (argv[1][0] == '-' && argv[1][1] == 'n' && argv[1][2] == '\0')
        {
            lineMax = atooi(argv[2]);
            fd = open(argv[3],O_RDONLY);
            if (fd < 0) 
            {
                write(fileno(stderr),strerror(errno),countChars(strerror(errno)));
                return 1;
            }
        }
        else
        {
            if (argv[2][1] != 'n' || argv[2][0] != '-' || argv[2][2] != '\0')
            {
                write(fileno(stderr),"did you mean -n?",20);
                return 1;
            }
            lineMax = atooi(argv[3]);
            fd = open(argv[1],O_RDONLY);
            if (fd < 0) 
            {
                write(fileno(stderr),strerror(errno),countChars(strerror(errno)));
                return 1;
            }

        }
        
    }
    else
    {
        write(fileno(stderr),"did you mean -n?",20);
        return 1;

    }
    while (read(fd,&buf,1)==1 && lineNo < lineMax)
    {        
        my_file_puts(1,buf);
        
        if (buf[0] == '\n')
        {
            lineNo++;
        }
        
    }
    close(fd);
    return 0;
}

int countChars(const char *s)
{
    int i = 0;
    while (s != NULL && s[i] != '\0')
    {
        i++;
    } 
    return i;
}
int my_file_puts(int fd, const char *s)
{
    int inputNumber = countChars(s);
    write(fd, s, inputNumber);
    return inputNumber;
}
int atooi(const char *s)
{
    int offset = 0 , n = 0;
    if (s[0] == '-') return n;
    for (int i = offset; s[i] != '\0'; i++) 
    {
        n = n * 10 + s[i] - '0';
    }   

    return n;
}



