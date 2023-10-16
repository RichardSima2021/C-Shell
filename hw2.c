#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <ctype.h>
#include <signal.h>
#include <fcntl.h>

#define bufferSize 80
#define whitespace " \t\n"
#define quit "quit"

int debug = 1;

void promptInput(char* inputBuffPtr){
    printf("%s", "prompt > ");
    fgets(inputBuffPtr, bufferSize, stdin);
}

void executeCommand(char* command, char** args, int argpos){
    if (debug){
        printf("argpos: %d\n", argpos);
        for(int i = 0; i < argpos; i++){
            printf("args: %s ", args[i]);
        }
        printf("\n");   
    }

    if(strcmp(command, "pwd") == 0){

    }
}

int main(){

    char* command = "";

    
    
    while(1){
        char inputBuffer[bufferSize]; 
        promptInput(inputBuffer);
        command = strtok(inputBuffer, whitespace);
        
        if(strcmp(command, quit) == 0){
            break;
        } else{
            if (debug) printf("%s\n", command); 

            // read first argument if exists
            char* arg = strtok(NULL, whitespace); // arg is a pointer to the first cell in a char array
            int argpos = 0;
            char* args[80] = {NULL}; // defines an array of 80 charpointers, these will store all the arguments of the command if there are any
            

            while(arg != NULL){
                args[argpos] = strdup(arg); // pointer to duplicated argument since arg will be overwritten by next strtok call
                argpos += 1;
                // continue reading arguments if exists
                arg = strtok(NULL, whitespace);
            }
            
            executeCommand(command, args, argpos);



        }

    }

    printf("%s\n","exit");
    return 0;
}