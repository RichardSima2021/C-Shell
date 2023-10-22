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

#define CHILD_NUMBER 1024

pid_t children[CHILD_NUMBER];
int child_count = 0;
pid_t fg_pid = 0;

void sigint_handler(int signo) {
    if (fg_pid > 0) {
        kill(fg_pid, SIGINT);
    }
}

void sigchld_handler(int signo) {
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

void terminate_all_children() {
    for (int i = 0; i < child_count; ++i) {
        kill(children[i], SIGTERM);  // or SIGKILL if you prefer a forceful termination
    }
}

int main() {
    char command[1024];
    char *argument;

    signal(SIGINT, sigint_handler);
    signal(SIGCHLD, sigchld_handler);

    while (1) {
        printf("prompt > ");
        fgets(command, sizeof(command), stdin);
        size_t cmd_len = strlen(command);
        if (command[cmd_len - 1] == '\n'){
            command[cmd_len - 1] = '\0';
            --cmd_len;
        }
        int is_bg = (command[cmd_len - 1] == '&');
        if (is_bg){
            command[cmd_len - 1] = '\0';
        }

        argument = strchr(command, ' ');  // Find the first space, if any
        if (argument) {
            *argument = '\0';  // Null-terminate the command
            argument++;  // Move to the next character
        }

        if (strcmp(command, "quit") == 0) {
            terminate_all_children();
            break;
        } else if (strcmp(command, "cd") == 0) {
            if (chdir(argument) != 0) {
                perror("No such file or directory.");
            }
        } else {
            pid_t pid = fork();

            if (pid < 0) {
                perror("Could not fork");
                exit(1);
            }
            if (pid == 0) {
                setpgid(0, 0); //process group id, set it to be itself
                char *args[] = {command, argument, NULL};
                execvp(args[0], args);
                perror("Execution failed");
                exit(1);
            } else {
                if (is_bg) {
                    children[child_count++] = pid;
                } else {
                    fg_pid = pid;
                    waitpid(pid, NULL, 0);
                    fg_pid = 0;
                }
            }
        }
    }

    return 0;
}

// ------------ stage 3 ---------------------