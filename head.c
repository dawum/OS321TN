#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

int my_file_puts(int fd,const char *s);
int countChars(const char *s);

char buf;  

int main(int argc, char **argv)
{
    int lineNo = 0, lineMax = 10;
    while (read(0,&buf,1)==1 && lineNo < lineMax)
    {        
        my_file_puts(1,&buf);
        
        if (buf == '\n')
        {
            lineNo++;
        }
        
    }
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
    if (fd==1)
    {
        write(fd, s, inputNumber);
    }
    return inputNumber;
}

