#include <string.h>
#include <stdio.h>
#include <unistd.h>


int main(){
    int i = 0;
    for(int c = 0; c < 5; c++){
        printf("Count: %d\n", c);
        sleep(1);
    }
    return 0;
}