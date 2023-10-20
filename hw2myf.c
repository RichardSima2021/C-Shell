// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <unistd.h>
// #include <sys/types.h>
// #include <sys/wait.h>
// #include <signal.h>
// #include <fcntl.h>

// pid_t pid = 0;

// void sigint_handler(int signo){
//     if (pid > 0) {
//         kill(pid, SIGINT);
//     }
// }

// int main() {
//     char command[50];
//     char argument[1024];
//     signal(SIGINT,sigint_handler);

//     while (5201314) {
//         printf("prompt > ");
//         scanf("%s", command);
//         if (strcmp(command, "cd") == 0) {
//             scanf("%s", argument);
//             if (chdir(argument) != 0) {perror("No such file or directory.");}
//         } 
//         else if (strcmp(command, "pwd") == 0) {
//             char cwd[1024];
//             if (getcwd(cwd, sizeof(cwd)) != NULL) {
//                 printf("%s\n", cwd);
//             } else {
//                 perror("Error getting current working directory");
//             }
//         } 
//         else if (strcmp(command, "quit") == 0) {
//             break;
//         } 
//         else {
//             pid = fork(); //#include <sys/types.h>
//             if (pid < 0){
//                 perror("Could not fork");
//                 exit(1);
//             } 
//             else if (pid == 0){
//                 char *args[] = {command, NULL};
//                 execvp(args[0], args); 
//                 perror("Failed to execute command");
//                 exit(1);

//             }
//             else{
//                 waitpid(pid, NULL, 0);
//             }
//         }
//         // Clear the input buffer
//         while ((getchar()) != '\n');
//     }


//     return 0;
// }

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

pid_t pid = 0;

void sigchld_handler(int signo) {
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

void sigint_handler(int signo) {
    if (pid > 0) {
        kill(pid, SIGINT);
    }
}

int main() {
    char command[50];
    char argument[1024];
    char input[1050];
    
    signal(SIGINT, sigint_handler);
    signal(SIGCHLD, sigchld_handler);

    while (1) {
        printf("prompt > ");
        fgets(input, sizeof(input), stdin);
        input[strcspn(input, "\n")] = 0;  // remove the newline
        
        if (strstr(input, "&")) {
            pid = fork();
            sscanf(input, "%s", command);
            if (pid == 0) {
                execlp(command, command, (char *) NULL);
                exit(EXIT_FAILURE);  // execlp failed
            }
            // don't wait for child process if it's a background job
        } else {
            sscanf(input, "%s %s", command, argument);
            
            if (strcmp(command, "cd") == 0) {
                if (chdir(argument) != 0) { perror("No such file or directory."); }
            } else if (strcmp(command, "pwd") == 0) {
                char cwd[1024];
                if (getcwd(cwd, sizeof(cwd)) != NULL) {
                    printf("%s\n", cwd);
                } else {
                    perror("Error getting current working directory");
                }
            } else if (strcmp(command, "quit") == 0) {
                break;
            } else {
                pid = fork();
                if (pid == 0) {
                    execlp(command, command, (char *) NULL);
                    exit(EXIT_FAILURE);  // execlp failed
                } else {
                    wait(NULL);  // wait for child process to finish
                }
            }
        }
    }

    return 0;
}

