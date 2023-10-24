#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main(int argc, char * argv[]){ 
    int i;

    printf( "Enter a value :");
    scanf("%d", &i);

    printf( "You entered: %d\n", i);
    return 0;
}