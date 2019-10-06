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
     
void qu_init(qu *q, size_t lm); 
void qu_clear(qu *q); 
static void qu_alloc_data(qu *q);
void qu_put(qu *q, char in);
void qu_push(qu *q);
void qu_clearData(qu *q);
static void qu_set_queue(qu *q);

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
    qu_set_queue(&q);
    while (read(fd,&buf,1)==1)
    {        
        qu_put(&q,buf[0]);
    }    
    for (int i = 0; i < qp->lineMax; i++)
    {
        write(1,qp->queue[i],countChars(qp->queue[i]));
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

void qu_init(qu *q, size_t lm)
{
    q->data = NULL;
    q->lineMax = lm;
    q->lineNo = 0;
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
    for (int i = 0; i < q->lineMax; i++)
    {
        if (q->queue[i] != NULL)
        {
            free(q->queue[i]);
        }
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
    q->data = NULL;
    q->lastIndex = 0;
    q->size = 0;
    q->remaining = 0;
}
static void qu_set_queue(qu *q)
{
    q->queue = (char **) calloc(q->lineMax,sizeof(char**));
     if (q->queue == NULL)
        {
            write(fileno(stderr),strerror(errno),countChars(strerror(errno)));
            exit(1);
        }
}
static void qu_alloc_data(qu *q)
{
    void *temp;
    if (q->data == NULL)
    {
        q->data = (char *) malloc(1 * sizeof(char));
        if (q->data == NULL)
        {
            write(fileno(stderr),strerror(errno),countChars(strerror(errno)));
            exit(1);
        }
        q->size = 1;
        q->remaining = 1;
    }
    else
    {
        if (q->remaining == 0)
        {
            temp = realloc(q->data, q->size + (1 * sizeof(char)));
            if (temp == NULL)
            {
                write(fileno(stderr),strerror(errno),countChars(strerror(errno)));
                exit(1);
            }
            q->data = (char *) temp;
            q->remaining = 1;
            q->size = q->size + 1;
        }
    }

}
void qu_put(qu *q, char in)
{
    qu_alloc_data(q);
    q->data[q->lastIndex] = in;
    q->remaining--;
    q->lastIndex++;
    if (in == '\n')
    {
        qu_push(q);
        qu_clearData(q);
    }
    
}
void qu_push(qu *q)
{
    if (q->lineNo < q->lineMax)
    {
        q->queue[q->lineNo] = q->data;
        q->lineNo++;  
    }
    else
    {
        void * temp = q->queue[0];
        for (int i = 0; i < q->lineMax-1; i++)
        {
            q->queue[i] = q->queue[i+1];

        }
        q->queue[q->lineMax-1] = q->data;
        free(temp);
    }
}


