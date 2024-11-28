#include <string.h>
#include <stdio.h>
#include <unistd.h>

#define BUFFER_SIZE 128

void print_welcome_message() {
    const char *message = "Bienvenue dans le Shell ENSEA.\nPour quitter, tapez 'exit'.\n";
    write(STDOUT_FILENO, message, strlen(message));
}

void print_prompt() {
    const char *prompt = "enseash % ";
    write(STDOUT_FILENO, prompt, strlen(prompt));
}

int main() {
    char buffer[BUFFER_SIZE];
    print_prompt();
    print_welcome_message();
    return 0;
}