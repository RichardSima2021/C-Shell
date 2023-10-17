#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

int main(){
    // demo of running an external program in the current one
    printf("Executing count from executeProgram\n");
    char* path = "count";
    char* args[] = {path, "arg1", NULL};
    execv("nonexistant", args);
    printf("Finished executing count\n");
    return 0;
}
