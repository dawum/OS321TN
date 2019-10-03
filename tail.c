#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

struct iamqueue
{
    char *data;
    char **queue;
    size_t lineNo;
    size_t lineMax;
    size_t lastIndex;
    size_t size;
    size_t remaining; 
}; typedef struct iamqueue qu;
     
void qu_init(qu *q, int lineNo); 
void qu_clear(qu *q); 
static void qu_alloc_data(qu *q);
void qu_put(qu *q, char in);
//size_t qu_queue_out(char out[], size_t n, qu *q);


int countChars(const char *s);
int my_file_puts(int fd,const char *s);

int atooi(const char *s);
char buf[1];  
size_t lineMax, lineNo; 
int fd;

int main(int argc, char **argv)
{
    qu q;
    qu *qp = &q;
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
           exit(1);
        }

    }
    else if (argc == 3)
    {
         if (argv[1][1] != 'n' || argv[1][0] != '-' || argv[1][2] != '\0')
            {
                write(fileno(stderr),"did you mean -n?",20);
                exit(1);
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
                exit(1);
            }
        }
        else
        {
            if (argv[2][1] != 'n' || argv[2][0] != '-' || argv[2][2] != '\0')
            {
                write(fileno(stderr),"did you mean -n?",20);
                exit(1);
            }
            lineMax = atooi(argv[3]);
            fd = open(argv[1],O_RDONLY);
            if (fd < 0) 
            {
                write(fileno(stderr),strerror(errno),countChars(strerror(errno)));
                exit(1);
            }

        }
        
    }
    else
    {
        write(fileno(stderr),"did you mean -n?",20);
        exit(1);

    }
    qu_init(&q,lineMax);
    while (read(fd,&buf,1)==1)
    {        
        qu_put(&q,buf[0]);
/*        if (buf[0] == '\n')
        {
            lineNo++;
        }
 */       
    }    
    for (int i = 0; i < qp->lastIndex+1; i++)
    {
        printf("%c",qp->data[i]);
    }
    close(fd);
    qu_clear(&q);
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

void qu_init(qu *q, int lineNo)
{
    q->data = NULL;
    q->lineNo = lineNo;
    q->queue = (char **) calloc(q->lineNo,sizeof(char*));
    if (q->queue == NULL)
        {
            write(fileno(stderr),strerror(errno),countChars(strerror(errno)));
            exit(1);
        }
    q->size = 0;
    q->lastIndex = 0;
    q->remaining = 0;
}
void qu_clear(qu *q)
{
    if (q->data != NULL) 
    {
        free(q->data);
        q->data = NULL;
    }
    if (q->queue != NULL) 
    {
        free(q->queue);
        q->queue = NULL;
    }
    q->lineNo = 0;
    q->lineMax = 0;
    q->lastIndex = 0;
    q->size = 0;
    q->remaining = 0;
}
void qu_clearData(qu *q)
{
}
static void qu_alloc_data(qu *q)
{
    void *temp;
    if (q->data == NULL)
    {
        q->data = (char *) calloc(2,sizeof(char));
        if (q->data == NULL)
        {
            write(fileno(stderr),strerror(errno),countChars(strerror(errno)));
            exit(1);
        }
        q->size = 2;
        q->remaining = 2;
    }
    else
    {
        if (q->remaining == 0)
        {
            temp = realloc(q->data, q->size * 2 * sizeof(char));
            if (temp == NULL)
            {
                write(fileno(stderr),strerror(errno),countChars(strerror(errno)));
                exit(1);
            }
            q->data = (char *) temp;
            q->remaining = (q->size * 2) - q->size;
            q->size = q->size * 2;
        }
    }
    printf("SIZE:%ld",q->size);

}
void qu_put(qu *q, char in)
{
    qu_alloc_data(q);
    if (q->lastIndex == 0)
    {
        q->data[q->lastIndex] = in;
        q->remaining--;
        q->lastIndex++;
    }
    else
    {
        q->data[q->lastIndex + 1] = in;
        q->remaining--;
        q->lastIndex++;
    }
}

size_t qu_queue_out(size_t n, qu *q)
{

}