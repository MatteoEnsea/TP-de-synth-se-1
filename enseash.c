#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>
#include <stdlib.h>
#include <fcntl.h>

#define BUFFER_SIZE 128 

void print_welcome_message() { 
    const char *message = "Bienvenue dans le Shell ENSEA.\nPour quitter, tapez 'exit'.\n";
    write(STDOUT_FILENO, message, strlen(message));
}

void print_prompt(int status,long time) { //Print the prompt with the return code of the previous  command
    char prompt[BUFFER_SIZE] = "enseah ";

    if (WIFEXITED(status)){//If the child process didn't exit normaly
        int exit_code=WEXITSTATUS(status);
        sprintf(prompt + strlen(prompt), "[exit:%d]|%ldms]  %% ", exit_code,time);
    }else if (WIFSIGNALED(status)){// If the child process was killed by a signal
        int signal = WTERMSIG(status);
        sprintf(prompt + strlen(prompt), "[signal:%d|%ldms] %% ", signal,time);
    }else{//If the child process exit normaly
        sprintf(prompt + strlen(prompt), "|%ldms  %% ",time);
    }

    write(STDOUT_FILENO, prompt, strlen(prompt));
}

long calculating_time(struct timespec start, struct timespec end){ //Compute time in ms between end and start
    long sec = end.tv_sec - start.tv_sec;
    long nsec = end.tv_nsec - start.tv_nsec;
    long time = sec*1000 + nsec/1000000;
    return time;
}

void parse_command(char *input, char *args[], char **input_file, char **output_file) { 
    //Split all inputs arguments in args, args[0] is the command or in input/output_file if > or <
    int i = 0;
    char *token = strtok(input, " ");
    *input_file = NULL;
    *output_file = NULL;

   while (token != NULL) {
       if (strcmp(token, "<") == 0) { //For stdin
           token = strtok(NULL, " ");
           if (token != NULL) {
               *input_file = token;
           }
       } else if (strcmp(token, ">") == 0) { //For stdout
           token = strtok(NULL, " ");
           if (token != NULL) {
               *output_file = token;
           }
       } else {
           args[i++] = token; // Normal arguments
       }
       token = strtok(NULL, " ");
   }
   args[i] = NULL; //Finish the array by NULL for execvp
}



int main() {
    char buffer[BUFFER_SIZE];
    char *args[BUFFER_SIZE/2];
    ssize_t read_size; 
    int last_status = 0; 
    long timems = 0;
    char *input_file, *output_file; 

    print_welcome_message();

    while (1){

        print_prompt(last_status,timems);

        read_size = read(STDIN_FILENO, buffer, BUFFER_SIZE - 1);

        if (read_size > 0 && buffer[read_size - 1] == '\n') {//Stocks and finish correctly the user's entry to use it later
            buffer[read_size - 1] = '\0';
        } 

        if (strncmp(buffer, "exit", 4) == 0){ //Exit the programm before using fork to kill father process
            write(STDOUT_FILENO,"À bientot...\n", strlen("À bientot...\n"));
            break;    
        }

        pid_t pid = fork();

        if (pid > 0){ //If father process, wait for the child process to finish, compute the time and update the status
            struct timespec tick,tack;
            int status;
            clock_gettime(CLOCK_MONOTONIC, &tick);
            wait(&status);
            clock_gettime(CLOCK_MONOTONIC, &tack);
            timems = calculating_time(tick,tack);
            last_status = status; //Update the status for the prompt

        }else if (pid==0){ //If child process, execute the command
            parse_command(buffer, args, &input_file, &output_file);

           if (input_file != NULL) { //Read only when <
               int input_fd = open(input_file, O_RDONLY);
               if (input_fd < 0) {
                   perror("Erreur lors de l'ouverture du fichier d'entrée");
                   _exit(1);
               }
               dup2(input_fd, STDIN_FILENO);
               close(input_fd);
           }

           if (output_file != NULL) { //Write only or create or overwrite when >
               int output_fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
               if (output_fd < 0) {
                   perror("Erreur lors de l'ouverture du fichier de sortie");
                   _exit(1);
               }
               dup2(output_fd, STDOUT_FILENO);
               close(output_fd);
           }
            execvp(args[0], args);
            perror("Erreur lors de l'exécution de la commande");
            _exit(1);

        }else if (pid==-1){ //If error using fork
            perror("Erreur lors du fork");
            return 1;
        }
    }
    return 0;
}