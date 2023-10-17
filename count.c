#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>


int main(int argc, char const * argv[]){
    int i = 0;
    int count = 5;
    if(argc > 1){
        printf("Count was called with argument: %s\n",argv[1]);
        count = atoi(argv[1]);
    }
    for(int c = 1; c <= count; c++){
        printf("Count: %d\n", c);
        sleep(1);
    }
    return 0;
}