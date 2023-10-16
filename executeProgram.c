#include <stdlib.h>
#include <stdio.h>

int main(){
    // demo of running an external program in the current one
    printf("Executing count from executeProgram\n");
    system("./count");
    printf("Finished executing count\n");
    return 0;
}
