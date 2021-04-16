#include <stdio.h>	// For: printf(), fprintf(), getchar(), perror(), stderr
#include <stdlib.h>     // For: malloc(), realloc(), free(), exit(), execvp(), EXIT_SUCCESS, EXIT_FAILURE
#include <string.h>     // For: strtok(), strcmp(), strcat(), strcpy()
#include <unistd.h>     // For: chdir(), fork(), exec(), pid_t, getcwd()
#include <sys/wait.h>	// For: waitpid()
#include <fcntl.h>
#include <time.h>
/*
 * Environment variables
 */
char PWD[1024];		// Present Working Directory
char PATH[1024];
char *builtin_str[] = {
  "cd",
  "exit"
};

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

int (*builtin_func[]) (char**) = {
    &my_cd,
    &my_exit
};


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

char *read_command_line(void){
    int position = 0;
    int buf_size = 1024;
    char *buffer = (char *)malloc(sizeof(char) * buf_size);
    char c;

    if (!buffer){
        fprintf(stderr, "lsh: allocation error\n");
        exit(EXIT_FAILURE);
    }
    while (1){
        c = getchar();

        if (c == EOF) {
            exit(EXIT_SUCCESS);
        } else if (c == '\n'){
            buffer[position] = '\0';
            return buffer;
        } else {
            buffer[position] = c;
        }
        position++;
        if (position >= buf_size){
                buf_size += 64;
                buffer = (char*)realloc(buffer, buf_size);
            if (!buffer){
                fprintf(stderr, "Allocation error\n");
                exit(EXIT_FAILURE);
            }
        }
    }
}

#define DELIM " \t\r\n\a"
char **split_line(char *buffer){
    int buf_size = 64;
    int position = 0;
    char **tokens = (char**)malloc(buf_size * sizeof(char*));
    char *token, **tokens_backup;
    if (!tokens){
        fprintf(stderr, "Allocation error\n");
        exit(EXIT_FAILURE);
    }

    token = strtok(buffer, DELIM);
    while(token != NULL) {
        tokens[position] = token;
        position++;

        if (position >= buf_size){
            buf_size += 64;
            tokens_backup = tokens;
            tokens = (char**)realloc(tokens, buf_size * sizeof(char*));
            if (!tokens) {
                free(tokens_backup);
                fprintf(stderr, "Allocation error\n");
                exit(EXIT_FAILURE);
            }
        }
        token = strtok(NULL, DELIM);
    }
    tokens[position] = NULL;
    return tokens;
}

int run_program(char **args, int is_bg){
    pid_t pid;
    int status;
    pid = fork();

    if (pid < 0){
        perror("Fork failed");

    }
    else if (pid != 0) {
        if (is_bg == 0){
            pid = wait(&status);
        }
        else {
            printf("[1] %d\n", getpid());
            waitpid(pid, &status, WNOHANG);
        }
    }
    else {
        if (execvp(args[0], args) == -1){
            perror("Wrong Command");
        }
        exit(EXIT_FAILURE);
    }
    return 1;
}

int redirect_program(char **args, int is_bg, int flag, char* input_val, char* output_val){
    int input_fd, output_fd;
    int status;
    pid_t pid;

    pid = fork();
    if (pid < 0){
        perror("Fork failed");
    }
    else if (pid != 0) {
        if (is_bg == 0){
            pid = wait(&status);
        }
        else{
            printf("[1] %d\n", getpid());
            waitpid(pid, &status, WNOHANG);
        }
    }
    else {
        if (flag == 1) {
            input_fd = open(input_val, O_RDONLY);
            if (input_fd < 0){
                perror("Wrong input redirection");
                exit(2);
            }
            dup2(input_fd, 0);
            close(input_fd);
            execvp(args[0], args);
        }
        else if (flag == 2){
            output_fd = open(output_val, O_CREAT|O_TRUNC|O_WRONLY, 0600);
            dup2(output_fd, 1);
            close(output_fd);
            execvp(args[0], args);
        }
        else {
            if (input_val != NULL && output_val != NULL){
                input_fd = open(input_val, O_RDONLY);
                if (input_fd < 0){
                    perror("Wrong input redirection");
                    exit(2);
                }
                dup2(input_fd, 0);
                close(input_fd);
                
                output_fd = open(output_val, O_CREAT|O_TRUNC|O_WRONLY, 0600);
                dup2(output_fd, 1);
                close(output_fd);

                execvp(args[0], args);
            }
        }
    }
    return 1;
}

int shell_execute(char **args){
    if (args[0] == NULL){
        return 1;
    }

    int flag = 0;
    int is_bg = 0;
    char* input_val;
    char* output_val;

    int i = 0;
    while(args[i] != NULL){
        if (strcmp(args[i], "<") == 0 && args[i+1] != NULL){
            if(flag == 0){
                flag = 1;
            }
            else{
                flag = 3;
            }
            input_val = args[i+1];
            args[i] = NULL;
            args[i+1] = NULL;
            i += 2;
        }
        else if (strcmp(args[i], ">") == 0 && args[i+1] != NULL){
            if(flag == 0){
                flag = 2;
            }
            else{
                flag = 3;
            }
            output_val = args[i+1];
            args[i] = NULL;
            args[i+1] = NULL;
            i += 2;
        }
        else if (strcmp(args[i], "&") == 0 && args[i+1] == NULL) {
            is_bg = 1;
            args[i] = NULL;
            i++;
        }
        else{
            i++;
        }
    }
    // TEST!!!!
    // printf("flag: %d\n", flag);
    // printf("is_bg: %d\n", is_bg);
    // printf("input: %s\n", input_val);
    // printf("output: %s\n", output_val);

    // int a = 0;
    // while (args[a] != NULL){
    //     printf("%s\n", args[a]);
    //     a++;
    // }
    // TEST!!!!!!

    for (i = 0; i < 2; i++){
        if (strcmp(args[0], builtin_str[i]) == 0){
            return (*builtin_func[i])(args);
        }
    }

    if (flag == 0){
        return run_program(args, is_bg);
    }
    else{
        return redirect_program(args, is_bg, flag, input_val, output_val);
    }

    return 1;
}

void shell_loop(void){
    char * command_line;
    char ** arguments;
	int status = 1;

    while (status){
        print_default();
        command_line = read_command_line();
        if ( strcmp(command_line, "") == 0 ){
            continue;
        }
        arguments = split_line(command_line);
        status = shell_execute(arguments);
    }
}


int main(int argc, char ** argv){
        // Shell initialization
	getcwd(PWD, sizeof(PWD));	// Initialize PWD Environment Variable
	strcpy(PATH, PWD);		// Initialize the command PATH

    // Main loop of the shell
    shell_loop();

    return 0;
}