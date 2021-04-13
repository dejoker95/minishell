#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <ctime>
#include <iostream>
#include <cstdio>
#include <string>
#include <time.h>
#include <vector>
#include <sstream>
#define MAXARG 7

char PWD[1024];

char* myfunc[] = {"cd", "exit"};
char* unix[] = {"ls", "mkdir", "echo", "cat", "head", "tail", "time", \
"rm", "pwd", "cp", "touch", "ps", "more", "top", "kill", "ln", "sleep"};

int my_cd(char** args){

    if (args[1] == NULL){
        char home[1024];
        strncpy(home, getenv("HOME"), sizeof(home));
        chdir(home);
    }
    else if (chdir(args[1]) < 0){
        perror("Wrong directory");
    }
    getcwd(PWD, sizeof(PWD));
    return 1;
}

int my_exit(char** args){
    return 0;
}

int (* my_function[]) (char**) = {
    &my_cd,
    &my_exit
};

void run_unix(char** arg, int background){
    int status, pid, out;
    pid = fork();

    if (pid < 0){
        perror("fork error");
    }
    else if(pid != 0){
        if(background == 0){
            pid = wait(&status);
        }
        else{
            printf("[1] %d\n", getpid());
            waitpid(pid, &status, WNOHANG);
        }
    }      
    else{
        out = open("/dev/null", O_RDONLY);
        dup2(out, 1);
        close(out);
        execvp(arg[0], arg);
    }
}

int run_program(int is_back, char** args){
    int status;
    pid_t pid;

    pid = fork();

    if (pid < 0){
        perror("fork error");
        return 1;
    }

    if (pid == 0){
        execv(args[0], args);
        return 1;
    }
    else{
        wait(NULL);
        return 1;
    }
}


char* read_command_line(void){
    int position = 0;
    int buf_size = 1024;
    char* command = (char*)malloc(sizeof(char) * buf_size);
    char c;

    // read command line by character
    c = getchar();
    while (c != EOF && c != '\n'){
        command[position] = c;

        // realloc buffer when needed
        if (position >= buf_size){
            buf_size += 64;
            command = (char*)realloc(command, buf_size);
        }

        position++;
        c = getchar();
    }
    return command;
}

char** split_command_line(char* command){
    int position = 0;
    int num_tokens = 64;
    char** tokens = (char**)malloc(sizeof(char*) * num_tokens);
    char delim[2] = " ";

    // split
    char* token = strtok(command, delim);
    while (token != NULL){
        tokens[position] = token;
        position++;
        token = strtok(NULL, delim);
    }
    tokens[position] = NULL;
    return tokens;
}

void print_default(void){
    //get username
    char username[1024];
    getlogin_r(username, sizeof(username));

    // get time
    time_t rawtime;
    struct tm* timeinfo;
    char buf[80];

    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(buf, sizeof(buf), "%X", timeinfo);

    // print default output
    printf("[%s]%s@%s$", buf, username, PWD);
}

int execute_command(char** args){
    char** arg = (char**)malloc(sizeof(char*)*MAXARG);
    int flag = 0;   // input redirect==1, output redirect==2, both==3
    int background = 0; // if background == 1
    char input[20];
    char output[20];

    // empty
    if (args[0] == NULL){
        return 1;
    }
    
    // check command
    int count = 0;
    int num = 0;
    while ( args[count] != NULL ){
        // input redirection check
        if ( strcmp(args[count], "<") ==0 ){
            if (flag == 0){
                flag = 1;
            }
            if (flag == 2){
                flag = 3;
            }
            strcpy(input, args[count+1]);
            count += 2;
        }
        // output redirection check
        else if (strcmp( args[count], ">") == 0){
            if (flag == 0){
                flag = 2;
            }
            if (flag == 1){
                flag = 3;
            }
            strcpy(output, args[count+1]);
            count += 2;
        }
        // background check
        else if ( strcmp(args[count], "&") == 0 && args[count+1] == NULL ){
            background = 1;
            count++;
        }
        else {
            arg[num++] = args[count++];
        }
    }
    // int p = 0;
    // while (arg[p] != NULL){
    //     printf("%s\n", arg[p++]);
    // }
    // cd, exit
    for(int i = 0; i < 2; i++){
        if ( strcmp(arg[0], myfunc[i]) == 0 ){
            int new_status = (* my_function[i])(arg);
            return new_status;
        }
    }
    
    // unix programs
    for(int i = 0; i < 16; i++){
        if ( strcmp(arg[0], unix[i]) == 0 ){
            run_unix(arg, background);
            return 1;
        }
    }
    
    // // other programs
    // run_program(arg);
    return 1;
}

void shell_start(void){
    int status = 1;
    
    char* command_line;
    char** arguments;

    while(status){
        // print default
        print_default();
        command_line = read_command_line();
        if ( strcmp(command_line, "") == 0 ){
            continue;
        }
        arguments = split_command_line(command_line);
        status = execute_command(arguments);
    }
}

// main function
int main(){
    getcwd(PWD, sizeof(PWD));
    // shell_start();
    shell_start();
    return 0; 
}





