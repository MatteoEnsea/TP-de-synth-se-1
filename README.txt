Dépot de fichier du TP de synthèse majeure informatique.
Un commit a été fait pour chaques questions.



#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>

#define BUFFER_SIZE 128 

void print_welcome_message() { 
    const char *message = "Bienvenue dans le Shell ENSEA.\nPour quitter, tapez 'exit'.\n";
    write(STDOUT_FILENO, message, strlen(message));
}

void print_prompt(int status,int time) { //Print the prompt with the return code of the previous  command
    char prompt[BUFFER_SIZE] = "enseah ";

    if (WIFEXITED(status)){//If the child process didn't exit normaly
        int exit_code=WEXITSTATUS(status);
        sprintf(prompt + strlen(prompt), "[exit:%d]|%ldms]  % ", exit_code,time);
    }else if (WIFSIGNALED(status)){// If the child process was killed by a signal
        int signal = WTERMSIG(status);
        sprintf(prompt + strlen(prompt), "[signal:%d|%ldms] % ", signal,time);
    }else{//If the child process exit normaly
        sprintf(prompt + strlen(prompt), "|%ldms  % ",time);
    }

    write(STDOUT_FILENO, prompt, strlen(prompt));
}

long calculating_time(struct timespec start, struct timespec end){ //Compute time in ms between end and start
    long sec = end.tv_sec - start.tv_sec;
    long nsec = end.tv_nsec - start.tv_nsec;
    long time = sec*1000 + nsec/1000000;
    return time;
}

void parse_command(char *input, char *args[]) {
    int i = 0;
    char *token = strtok(input, " "); // Séparer les arguments par espace
    while (token != NULL) {
        args[i++] = token;
        token = strtok(NULL, " ");
    }
    args[i] = NULL; // Terminer le tableau par NULL pour execvp
}



int main() {
    char buffer[BUFFER_SIZE];
    ssize_t read_size; 
    int last_status = 0; 
    long timems = 0;

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

        if (pid > 0){ //If father process, wait for the child process to finish
            struct timespec tick,tack;
            int status;
            clock_gettime(CLOCK_MONOTONIC, &tick);
            wait(&status);
            clock_gettime(CLOCK_MONOTONIC, &tack);
            timems = calculating_time(tick,tack);
            last_status = status; //Update the status for the prompt
        }else if (pid==0){ //If child process, execute the command
            // Processus enfant : exécuter la commande
            parse_command(buffer, args);
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
