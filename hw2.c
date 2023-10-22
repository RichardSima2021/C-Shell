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

struct Job{
    // each child process owns a struct
    int jobId; // assigned by shell
    int processId; // returned to parent

    // 1 = fg
    // 0 = bg
    int foreground;
    // Running
    // Stopped
    char* status;
    char** args;
    int numargs;
};  

struct Job* jobList[5] = {NULL};
int latestJobId = 1;
int nextJobIndex = 0;
int foregroundStopped = 0;

void cleanJobList(int endedPid){
    // removes the job within joblist of the job that just ended
    // shifts nextJobIndex back by 1
    // free each argument, then free the argument array, then free the struct
    int removeIndex;
    for(int i = 0; i < nextJobIndex; i++){
        if (jobList[i] -> processId == endedPid){
            removeIndex = i;
        }
    }

    for(int a = 0; a < jobList[removeIndex] -> numargs; a++){
        free(jobList[removeIndex] -> args[a]); 
    }

    free(jobList[removeIndex] -> args);
    free(jobList[removeIndex]);
    jobList[removeIndex] = NULL;

    for(int i = 0; i < 4; i++){
        // shift everything in jobList back by 1 after removing a job;
        if(jobList[i] == NULL && jobList[i+1] != NULL){
            jobList[i] = jobList[i+1];
            jobList[i+1] = NULL;
        }
    }

    // next available index in jobList goes back by 1
    nextJobIndex -= 1;
}

void jobStatusStopped(int stoppedPid){
    // updates the job with stoppedPid's status to stopped
    for(int i = 0; i < nextJobIndex; i++){
        if(jobList[i] -> processId == stoppedPid){
            jobList[i] -> status = "Stopped";
            jobList[i] -> foreground = 0;
            break;
        }
    }
}

void nonResponseINTHandler(int sig){
    if (debug) printf("Handled SIGINT from %d\n", getpid());
}

void nonResponseTSTPHandler(int sig){
    if (debug) printf("Handled TSTP from %d\n", getpid());
}

void shellTSTPHandler(int sig){
    if (debug) printf("Handling TSTP from shell\n");
}

void SIGCHLDHandler(int sig){
    if (debug) printf("SIGCHLD handler called\n");
    int child_status;
    int waitpid_status = waitpid(-1, &child_status, WNOHANG); 
    if (debug) printf("waitpid called\n");
    if (debug){
        if(waitpid_status < 0){
            perror("waitpid");
        } else if(waitpid_status > 0){
            printf("Child pid: %d stopped or terminated\n", waitpid_status);
        } else{
            printf("No children to be reaped\n");
        }
    }
    
    if(waitpid_status > 0){
        if (WIFEXITED(child_status)){
            if (debug) printf("Child finished and reaped\n");
            // remove from job list
            cleanJobList(waitpid_status);
        } else if (WIFSIGNALED(child_status)){
            if (debug) printf("Child was terminated and reaped\n");
            // remove from job list
            cleanJobList(waitpid_status);
        } else if (WIFSTOPPED(child_status)){
            if (debug) printf("Child was stopped\n");
        }
    }
}

void nonResponseCHLDHandler(int sig){
    if (debug) printf("PID: %d encountered a SIGCHLD signal and ignored it\n", getpid());
} 


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
    if(numargs < 1 || (numargs == 1 && strcmp(args[1],"~") == 0)){
        // if no arguments or argument = "~"
        char *homeDirectory = getenv("HOME");
        chdir(homeDirectory);
    } else if(numargs > 2){
        // if too many arguments (>1)
        printf("Error: Too many arguments");
    } else{
        // fix
        // if 1 argument
        if(chdir(args[1]) != 0){
            // if changing directory throws an error
            perror("Error");
        }
        // otherwise changing directory was successful
    }
}

void executeTaskFg(char* programName, char** args, int numargs){
    pid_t pid = fork();

    if(pid == 0){
        signal(SIGINT, SIG_DFL); // fg task must be able to be affected by Ctrl+C
        signal(SIGTSTP, SIG_DFL); // fg task must be able to be affected by Ctrl+Z
        // signal(SIGUSR2, foregroundUSR2Handler);
        // signal(SIGCHLD, nonResponseCHLDHandler); 
        if (debug) printf("Forked child and running fg process: %s pid: %d, pgid: %d\n", programName, getpid(), getpgid(getpid()));
        // child
        if(execv(programName, args)){
            // if there's a return value at all
            printf("%s: Program not found.\n", programName);
            exit(0);
        }
        kill(getppid(), SIGUSR1);
    } else{
        // parent, pid = process id of child
        int child_status;
        if (debug) printf("shell waiting for foreground task to complete\n");

        struct Job* newForeroundJob = malloc(sizeof(struct Job)); // will need to free later in SIGCHLD handler or smth
        newForeroundJob -> jobId = latestJobId;
        newForeroundJob -> processId = pid;
        newForeroundJob -> foreground = 1;
        newForeroundJob -> status = "Running";
        // newBackgroundJob -> args = args;
        newForeroundJob -> numargs = numargs;

        if (debug) printf("Duplicating the following arguments: ");
        char** argsCopy = malloc((numargs + 2) * sizeof(char*));
        for (int i = 0; i < numargs + 1; i++){
            if (debug) printf("%s ", args[i]);
            argsCopy[i] = strdup(args[i]);
        }
        argsCopy[numargs + 1] = NULL;
        newForeroundJob -> args = argsCopy; // will need to free the array and each string later


        jobList[nextJobIndex] = newForeroundJob;
        if (debug) printf("\nCreating a new job and insert into jobList at index %d\n", nextJobIndex);
        nextJobIndex += 1;
        latestJobId += 1;
        
        int waitpid_status = waitpid(pid, &child_status, WUNTRACED); 
        
        if (debug) printf("waitpid of foreground process called\n");

        if (WIFEXITED(child_status)){
            if (debug) printf("Foreground child finished and reaped\n");
            cleanJobList(waitpid_status);
        } else if (WIFSIGNALED(child_status)){
            if (debug) printf("Foreground child was terminated and reaped\n");
            cleanJobList(waitpid_status);
        } else if (WIFSTOPPED(child_status)){
            if (debug) printf("Foreground child was stopped\n");
            jobStatusStopped(waitpid_status);
        }
    }
}

void executeTaskBg(char* programName, char** args, int numargs){
    if (debug) printf("called execute task in background with program: %s\n", programName);
    pid_t pid = fork();
    if(pid == 0){
        // child
        if(setpgid(getpid(),getpid()) == -1){
            // error setting pgid to its own pid
            perror("setpgid");
        }
        signal(SIGINT, nonResponseINTHandler); // should not respond to any SIGINT
        signal(SIGTSTP, nonResponseTSTPHandler); // should not respond to any SIGTSTP
        // signal(SIGCHLD, nonResponseCHLDHandler); // should not respond to any SIGCHLD
        if (debug) printf("Forked child and running bg process: %s, pid: %d, pgid: %d\n", programName, getpid(), getpgid(getpid()));
        if(execv(programName, args)){
            perror("execv");
            exit(EXIT_FAILURE);
        }
    } else{
        
        struct Job* newBackgroundJob = malloc(sizeof(struct Job)); // will need to free later in SIGCHLD handler or smth
        newBackgroundJob -> jobId = latestJobId;
        newBackgroundJob -> processId = pid;
        newBackgroundJob -> foreground = 0;
        newBackgroundJob -> status = "Running";
        // newBackgroundJob -> args = args;
        newBackgroundJob -> numargs = numargs;

        if (debug) printf("Duplicating the following arguments: ");
        char** argsCopy = malloc((numargs + 2) * sizeof(char*));
        for (int i = 0; i < numargs + 1; i++){
            if (debug) printf("%s ", args[i]);
            argsCopy[i] = strdup(args[i]);
        }
        argsCopy[numargs + 1] = NULL;
        newBackgroundJob -> args = argsCopy; // will need to free the array and each string later

        jobList[nextJobIndex] = newBackgroundJob;
        if (debug) printf("\nCreating a new job and insert into jobList at index %d\n", nextJobIndex);
        nextJobIndex += 1;
        latestJobId += 1;
    }
}

void killJob(char** args, int numargs){
    int passedByJobID;
    if (args[1][0] == '%'){
        printf("Killing job with jobid: %s", args[1]);
    } else{
        printf("Killing job with processid: %s", args[1]);
    }
}

void printJob(struct Job* jobPtr){
    struct Job job = *jobPtr;
    printf("[%d] ", job.jobId);
    printf("(%d) ", job.processId);
    printf("%s ", job.status);
    printf("%s", job.args[0]);
    for(int i = 1; i < job.numargs + 1; i++){
        printf(" %s", job.args[i]);
    }
    printf("\n");
}

void executeCommand(char* command, char** args, int numargs){
    if (debug){
        printf("numargs: %d\nargs: ", numargs);
        for(int i = 1; i < numargs + 1; i++){
            printf("%s ", args[i]);
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
        if(numargs == 0){
            printf("Error: no jobid or processid provided");
        } else {
            killJob(args, numargs);
        }
    } else if(strcmp(command, "jobs") == 0){
        // print all jobs from jobList
        int jobIndex = 0;
        while(jobList[jobIndex] != NULL){
            printJob(jobList[jobIndex]); // fill in printJob later
            jobIndex += 1;
        }
    } else if(strcmp(command, "println") == 0){
        printf("-------------------------------------------------\n");
    } else{
         if (debug) printf("Execute task");
        // start executing a program in fore/background
        if(numargs > 0 && strcmp(args[numargs], "&") == 0){
            // check if last arg is an "&"
            if (debug) printf(" in background\n");
            executeTaskBg(command, args, numargs);
        } else{
            if (debug) printf(" in foreground\n");
            executeTaskFg(command, args, numargs);
        }
    }
    
}

int main(){
    if (debug) printf("shell started in gpid: %d\n", getpgid(getpid()));
    signal(SIGINT, nonResponseINTHandler);
    signal(SIGTSTP, shellTSTPHandler);
    signal(SIGCHLD, SIGCHLDHandler);
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
            // defines an array of 80 charpointers [command, arg1, arg2, ...]
            char* args[80] = {NULL};   
            // store command into first element
            args[0] = command;

            // read first argument if exists
            char* arg = strtok(NULL, whitespace); // arg is a pointer to the first cell in a char array
            int argpos = 1;

            while(arg != NULL){
                args[argpos] = strdup(arg); // pointer to duplicated argument since arg will be overwritten by next strtok call
                argpos += 1;
                // continue reading arguments if exists
                arg = strtok(NULL, whitespace);
            }
            
            // argpos - 1 = actual number of arguments (excludes command) aka numargs in function
            // example:
            // prompt > count 5 &
            // command = "count"
            // args = ["count", "5", "&"]
            // argpos = 2
            executeCommand(command, args, argpos - 1);
        }

    }

    printf("%s\n","exit");
    return 0;
}