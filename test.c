#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

int main(int argc, char **argv)
{  
    const char *s = "hello mo";
    int i = 0;
    while (s)
    {
        if (s[i] != '\0')
        i++;
        else break;
    }
    write(1,s,i);
    
    return 0;
}
