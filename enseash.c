#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

#define BUFFER_SIZE 128 

void print_welcome_message() { 
    const char *message = "Bienvenue dans le Shell ENSEA.\nPour quitter, tapez 'exit'.\n";
    write(STDOUT_FILENO, message, strlen(message));
}

void print_prompt(int status) { //Print the prompt with the return code of the previous  command
    char prompt[BUFFER_SIZE] = "enseah ";

    if (WIFEXITED(status)){//If the child process didn't exit normaly
        int exit_code=WEXITSTATUS(status);
        sprintf(prompt + strlen(prompt), "[exit:%d] % ", exit_code);
    }else if (WIFSIGNALED(status)){// If the child process was killed by a signal
        int signal = WTERMSIG(status);
        sprintf(prompt + strlen(prompt), "[signal:%d] % ", signal);
    }else{//If the child process exit normaly
        sprintf(prompt + strlen(prompt), " % ");
    }

    write(STDOUT_FILENO, prompt, strlen(prompt));
}


int main() {
    char buffer[BUFFER_SIZE];
    ssize_t read_size; 
    int last_status = 0; 

    print_welcome_message();

    while (1){

        print_prompt(last_status);

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
            int status;
            wait(&status);
            last_status = status; //Update the status for the prompt
        }else if (pid==0){ //If child process, execute the command
            execlp(buffer,buffer,NULL);
            perror("Erreur lors de l'execution de la commande");
            _exit(1);
        }else if (pid==-1){ //If error using fork
            perror("Erreur lors du fork");
            return 1;
        }
    }
    return 0;
}