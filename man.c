#include "linkedlist.h"
#include "emalloc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <unistd.h>
#include <stddef.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>

#define MAX_DIR_LENGTH 1024
#define MAX_NAME 256

/**
 * @brief Prints the list of processes.
 * 
 * @param list The head of the linked list of processes.
 */
void print_list(node_t *list) {
    int count = 1;
    for (; list != NULL; list = list->next) {
        printf("%d: %s  %d\n", list->pid, list->command, count++);
    }
    printf("Total background jobs: %d\n", count-1);
}

/**
 * @brief Checks if a string is a number.
 * 
 * @param str The string to check.
 * @return true if the string is a number, false otherwise.
 */
bool string_is_number(char *str) {
    for (int i = 0; i < strlen(str); i++) {
        if (str[i] < '0' || str[i] > '9') {
            return false;
        }
    }
    return true;
}

/**
 * @brief Splits a string into words by spaces.
 * 
 * @param line The string to split.
 * @param count A pointer to an integer where the number of words will be stored.
 * @return A dynamically allocated array of words.
 */
char** split_string(const char* line, int* count) {
    char* copy = strdup(line);
    if (copy == NULL) {
        *count = 0;
        return NULL;
    }

    char** words = NULL;
    int word_count = 0;
    char* token = strtok(copy, " "); 

    while (token != NULL) {
        char* word = strdup(token);
        if (word == NULL) {
            continue;
        }

        word_count++;
        words = realloc(words, sizeof(char*) * word_count);
        words[word_count - 1] = word;

        token = strtok(NULL, " "); 
    }

    *count = word_count;
    free(copy); // Free the copied string
    return words;
}

/**
 * @brief Frees a list of words.
 * 
 * @param words The list of words to free.
 * @param count The number of words in the list.
 */
void free_words(char** words, int count) {
    for (int i = 0; i < count; i++) {
        free(words[i]);
    }
    free(words);
}

/**
 * @brief Executes a command using execvp.
 * 
 * @param is_bg Whether the command should be run in the background.
 * @param commands The command and its arguments.
 * @param p_list The head of the linked list of processes.
 * @return The head of the updated linked list of processes.
 */
node_t *execute_cmd(bool is_bg, char **commands, node_t *p_list){
    int pid = fork();
    if (pid < 0) { 
		fprintf(stderr, "Fork failed!!\n");
        return p_list;
	}
    if(pid == 0){
        int stat_code = execvp(commands[0], commands);
        if (stat_code == -1){
            fprintf(stderr, "Error: execution of %s failed\n", commands[0]);
            exit(EXIT_FAILURE);
        }
        exit(EXIT_SUCCESS);
    }else{ 
        if (!is_bg){
            waitpid(pid, NULL, 0);
            return p_list;
        }else{
            usleep(10000); // waite for a bit to see if the process will fail
            int wait_status;
            waitpid(pid, &wait_status, WNOHANG);
            if (WEXITSTATUS(wait_status) != EXIT_FAILURE) {
                node_t *node = new_node(pid, commands[0]);
                node_t *new_list = add_end(p_list, node);
                return new_list;
            }
            return p_list;
        }
    }
}

/**
 * @brief Checks if a command to modify a background process is valid.
 * 
 * @param commands The command and its arguments.
 * @param len_commands The number of arguments.
 * @return The process ID if the command is valid, -1 otherwise.
 */
int valid_modefy_process_command(char **commands, int len_commands){
    if (len_commands == 1){
        fprintf(stderr, "%s: p_id argument is required\n", commands[0]);
    } else if (len_commands > 2){
        fprintf(stderr, "%s: too many arguments\n", commands[0]);
    } else if (!string_is_number(commands[1])){
        fprintf(stderr, "%s: p_id argument must be a number\n", commands[0]);
    } else {
        return atoi(commands[1]);
    }
    return -1;
}

/**
 * @brief Changes the current directory.
 * 
 * @param commands The command and its arguments.
 * @param len_commands The number of arguments.
 */
void change_directory(char **commands, int len_commands){
    if (len_commands > 2){
        fprintf(stderr, "cd: too many arguments\n");
    } else if (len_commands == 1 || strcmp(commands[1], "~") == 0) {
        chdir(getenv("HOME"));
    } else if (commands[1][0] == '~' && commands[1][1] == '/') {
        char *home_dir = getenv("HOME");
        char *folder_path = emalloc(strlen(home_dir) + strlen(commands[1]));
        strcpy(folder_path, home_dir);
        strcat(folder_path, commands[1] + 1);
        int res = chdir(folder_path);
        if (res == -1){
            perror("getcwd(): error");
        }
        free(folder_path);
    } else {
        int res = chdir(commands[1]);
        if (res == -1){
            perror("getcwd(): error");
        }
    }
}

/**
 * @brief Executes a command.
 * 
 * @param commands The command and its arguments.
 * @param len_commands The number of arguments.
 * @param p_list The head of the linked list of processes.
 * @return The head of the updated linked list of processes.
 */
node_t *apply_command(char **commands, int len_commands, node_t *p_list){
    bool is_bg = false;
    if (strcmp(commands[0], "cd") == 0 || 
        (strcmp(commands[0], "bg") == 0 && strcmp(commands[1], "cd") == 0)
        ){
        if (strcmp(commands[0], "bg") == 0){
            commands = &commands[1];
            len_commands--;
        }
        change_directory(commands, len_commands);
    }
    else if (strcmp(commands[0], "bglist") == 0){
        if (len_commands > 1){
            fprintf(stderr, "bglist: too many arguments\n");
        }else{
            print_list(p_list);
        }
    }
    else if (strcmp(commands[0], "bgkill") == 0){
        int p_id = valid_modefy_process_command(commands, len_commands);
        if (p_id != -1){
            if (kill(p_id, SIGKILL) == -1){
                fprintf(stderr, "bgkill: Failed to kill/find process with p_id %d\n", p_id);
            }else{
                printf("Process with p_id %d has been killed\n", p_id);
                waitpid(p_id, NULL, 0); // Wait for the process to be removed
                p_list = remove_p_node(p_id, p_list);
            }
        } // if it is == -1, the function valid modefy process command has already printed the error
    }
    else if (strcmp(commands[0], "bgstop") == 0){
        int p_id = valid_modefy_process_command(commands, len_commands);
        if (p_id != -1){
            if (kill(p_id, SIGSTOP) == -1){
                fprintf(stderr, "bgstop: Failed to stop/find process with p_id %d\n", p_id);
            }else{
                printf("Process with p_id %d has been stopped\n", p_id);
            }
        } 
    }
    else if (strcmp(commands[0], "bgstart") == 0){
        int p_id = valid_modefy_process_command(commands, len_commands);
        if (p_id != -1){
            if (kill(p_id, SIGCONT) == -1){
                fprintf(stderr, "bgsart: Failed to start/find process with p_id %d\n", p_id);
            }else{
                printf("Process with p_id %d has been started\n", p_id);
            }
        }
    }
    else if (strcmp(commands[0], "bg") == 0){
        // A BACKGROUND COMMAND
        is_bg = true;
        for (int i = 0; i < len_commands-1; i++) {
            commands[i] = commands[i+1];
        }
        commands[len_commands-1] = NULL;
        p_list = execute_cmd(is_bg, commands, p_list);
    }
    else{
        // A NORMAL COMMAND
        commands = realloc(commands, sizeof(char*) * ++len_commands);
        commands[len_commands - 1] = NULL;
        p_list = execute_cmd(is_bg, commands, p_list);
    }

    return p_list;
}

/**
 * @brief Checks if any background processes have finished.
 * 
 * @param p_list The head of the linked list of processes.
 * @return The head of the updated linked list of processes.
 */
node_t *check_bg_processes(node_t *p_list) {
    node_t *p = p_list;
    while (p != NULL) {
        int wait_status;
        int res = waitpid(p->pid, &wait_status, WNOHANG);
        if(res == -1){
            printf("Command %s with pid %d has finished\n", p->command, p->pid);
            p_list = remove_p_node(p->pid, p_list);
        }
        p = p->next;
    }
    return p_list;
}

int main(){
    node_t *p_list = NULL;
    char hostname[MAX_DIR_LENGTH];
    char *username = getlogin();
    gethostname(hostname, sizeof(hostname));

    char current_directory[MAX_DIR_LENGTH];
    while(true){
        getcwd(current_directory, sizeof(current_directory));
        char prompt[2*MAX_DIR_LENGTH+2*MAX_NAME];
        sprintf(prompt, "%s@%s: %s > ", username, hostname, current_directory);
        const char *input_line = readline(prompt);
        int word_count;
        char** words = split_string(input_line, &word_count);
        if (word_count == 0  || (word_count == 1 && strcmp(words[0], "bg") == 0)){
            continue;
        }else if(strcmp(words[0], "exit") == 0 || strcmp(words[0], "bye") == 0){
            if (word_count > 1){
                fprintf(stderr, "exit: too many arguments\n");
            }else{
                free(input_line);
                free_words(words, word_count);
                printf("Good Bye\n");
                exit(EXIT_SUCCESS);
            }
        }
        p_list = apply_command(words, word_count, p_list);
        usleep(10000);
        p_list = check_bg_processes(p_list);

        free(input_line);
        free_words(words, word_count);
        usleep(1000);
    }
}
