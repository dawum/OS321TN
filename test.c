#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>

int main(int argc, char **argv)
{  
    int * array = calloc(2, sizeof(int));
    array[0] = 1;
    array[1] = 2;
    int *ggs = array;
    array = calloc(2, sizeof(int)); 
    array[0] = 1;
    array[1] = 2;
    printf("array = %d\n",array[0]);
    printf("ggs = %d\n",ggs[1]);   
    free(ggs);
    free(array);
    printf("Address of array = %p, address of ggs = %p",array,ggs);
    return 0;
}
