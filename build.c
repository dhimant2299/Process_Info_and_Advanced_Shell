#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h> 

#define MAX_COMMAND_LENGTH 1024
#define MAX_ARGS 64

// The structure stores information about a command
typedef struct {
    char **args;
    int num_args;
    int background; // Flag indicating background execution (0: foreground, 1: background)
} command_t;

// Function prototypes
void parse_command(char *command, command_t *commands, int *num_commands);
void free_arguments(command_t *commands, int num_commands);
void execute_command(command_t *cmd, int in_fd, int out_fd);

int main() {
    char input[MAX_COMMAND_LENGTH];
    command_t commands[MAX_ARGS];
    int num_commands = 0;
    int in_fd = STDIN_FILENO; // (stdin)
    int out_fd = STDOUT_FILENO; // (stdout)

    while (1) {
        printf("COSC-6325/456Shell$ ");
        if (fgets(input, sizeof(input), stdin) == NULL) {
            break;
        }

        input[strcspn(input, "\n")] = '\0'; // Removes trailing newline

        parse_command(input, commands, &num_commands);
        if (num_commands == 0) {
            fprintf(stderr, "shell: syntax error\n");
            continue;
        }

        // Check for background execution: where command ends with '&')
        if (commands[num_commands - 1].background) {
            commands[num_commands - 1].background = 0; // Reset flag for next iteration
            num_commands--; // Does not execute the background process in the main loop
        }

        // Handling input/output redirection
        for (int i = 0; i < num_commands; i++) {
            // Input redirection
            if (strchr(commands[i].args[0], '<') != NULL) {
                char *filename = strchr(commands[i].args[0], '<') + 1;
                commands[i].args[0] = strtok(commands[i].args[0], "<"); // Separates command and filename
                in_fd = open(filename, O_RDONLY); // Opens the input file
                if (in_fd == -1) {
                    perror("open");
                    break; // Exits the loop on error
                }
            }

            // Built-in "exit" command**
            if (strcmp(commands[i].args[0], "exit") == 0) {
                break; // Exits the program immediately "exit"
            }

            // Output redirection
            if (strchr(commands[i].args[0], '>') != NULL) {
                char *filename = strchr(commands[i].args[0], '>') + 1;
                commands[i].args[0] = strtok(commands[i].args[0], ">"); // Separate command and filename
                out_fd = open(filename, O_WRONLY | O_CREAT, 0644); // Open output file (create if it doesn't exist)
                if (out_fd == -1) {
                    perror("open");
                    break; // Exits the loop on error
                }
            }
        }

        // Exits the program immediately after "exit"
        if (strcmp(commands[0].args[0], "exit") == 0) {
            break;
        }

        // It executes each command in the sequence
        for (int i = 0; i < num_commands; i++) {
            execute_command(&commands[i], in_fd, out_fd);
            // Restores the default file descriptors for next command
            in_fd = STDIN_FILENO;
            out_fd = STDOUT_FILENO;
        }

        free_arguments(commands, num_commands);
        num_commands = 0;
    }

    return 0;
}

// Function that parses the input command
void parse_command(char *command, command_t *commands, int *num_commands) {
    char *token;
    int i = 0;

    token = strtok(command, "|");
    while (token != NULL) {
        commands[i].args = malloc(MAX_ARGS * sizeof(char *));
        commands[i].num_args = 0;
        commands[i].background = 0;
        char *arg = strtok(token, " ");
        while (arg != NULL) {
            commands[i].args[commands[i].num_args++] = arg;
            arg = strtok(NULL, " ");
        }
        commands[i].args[commands[i].num_args] = NULL; // It sets the last element to NULL for execvp
        if (commands[i].num_args > 0 && strcmp(commands[i].args[commands[i].num_args - 1], "&") == 0) {
            commands[i].background = 1; // Sets background flag if the command ends with '&'
            commands[i].args[commands[i].num_args - 1] = NULL; // Removes '&' from the args array
        }
        (*num_commands)++;
        i++;
        token = strtok(NULL, "|");
    }
}

// The Function helps to free allocated memory for command arguments
void free_arguments(command_t *commands, int num_commands) {
    for (int i = 0; i < num_commands; i++) {
        free(commands[i].args);
    }
}

// Function to execute a single command
void execute_command(command_t *cmd, int in_fd, int out_fd) {
    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    if (pid == 0) { // Child process
        if (in_fd != STDIN_FILENO) {
            dup2(in_fd, STDIN_FILENO); 
            close(in_fd);
        }
        if (out_fd != STDOUT_FILENO) {
            dup2(out_fd, STDOUT_FILENO); 
            close(out_fd);
        }
        execvp(cmd->args[0], cmd->args); // Executes the command
        perror("execvp"); 
        exit(EXIT_FAILURE);
    } else { // Parent process
        if (!cmd->background) {
            int status;
            waitpid(pid, &status, 0); // Waits for the child process if it's a foreground command
        }
    }
}// end
