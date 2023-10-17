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

// implementation for pwd shell command
void pwd(){
    char* dir = getcwd(NULL, 0);
    printf("%s\n", dir);
    free(dir);
}

// implementation for cd shell command
void cd(char ** args, int numargs){
    // cd args can be:
    // {NULL, "~", f"{someDirectory}"}
    if(numargs < 1 || (numargs == 1 && strcmp(args[0],"~") == 0)){
        // if no arguments or argument = "~"
        char *homeDirectory = getenv("HOME");
        chdir(homeDirectory);
    } else if(numargs > 1){
        // if too many arguments (>1)
        printf("Error: Too many arguments");
    } else{
        // if 1 argument
        if(chdir(args[0]) != 0){
            // if changing directory throws an error
            perror("Error");
        }
        // otherwise changing directory was successful
    }
}

void executeTaskFg(char* programName, char** args, int numargs){
    pid_t pid = fork();

    if(pid == 0){
        // child
        if(execv(programName, args)){
            // if there's a return value at all
            printf("%s: Program not found.\n", programName);
            exit(0);
        }
    } else{
        // parent, pid = process id of child
        int child_status;
        waitpid(pid, &child_status, 0);
        if (debug) printf("Child finished and reaped\n");
    }

}

void executeCommand(char* command, char** args, int numargs){
    if (debug){
        printf("numargs: %d\n", numargs);
        for(int i = 0; i < numargs; i++){
            printf("args: %s ", args[i]);
        }
        printf("\n");   
    } 
    
    if(strcmp(command, "pwd") == 0){
        pwd();
    } else if(strcmp(command, "cd") == 0){
        cd(args, numargs);
    } else if(strcmp(command, "fg") == 0){
        // change job that is in stopped or background/running to fg/running
        // job is identified via %+job_id or pid
    } else if(strcmp(command, "bg") == 0){
        // change job that is in stopped to background/running
        // same identifiers as fg
    } else if(strcmp(command, "kill") == 0){
        // kill job and reap | use kill() syscall with -9 signal to kill process
        // same identifiers
    } else{
         if (debug) printf("Execute task");
        // start executing a program in fore/background
        if(numargs > 0 && strcmp(args[0], "&") == 0){
            if (debug) printf(" in background\n");
            // execute background
        } else{
            if (debug) printf(" in foreground\n");
            executeTaskFg(command, args, numargs);
        }
    }
    
}

int main(){

    char* command = "";


    
    while(1){
        char inputBuffer[bufferSize]; 
        promptInput(inputBuffer);

        // empty line, do nothing
        if(strcmp(inputBuffer, "\n") == 0) continue;
        
        command = strtok(inputBuffer, whitespace);
        
        if(strcmp(command, quit) == 0){
            break;
        } else{
            if (debug) printf("command: %s\n", command); 

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