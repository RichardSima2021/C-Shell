#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int debug = 0;
int main(int argc, char const * argv[]){
    // printf("A job with no other output that just runs for 10 seconds\n");
    int i = 0;
    int count = 10;
    for(int c = 1; c <= count; c++){
        sleep(1);
    }
    printf("a finished running\n");
    return 0;
}