#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <time.h>
#include <sys/time.h>
#include <errno.h>
#include <signal.h>

#define MAXHISTORY 100
#define MAXCMDS 100

typedef struct {
    char *command;
    pid_t pid;
    time_t start_time;
    double duration;
} HistoryEntry;

HistoryEntry history[MAXHISTORY]; 
int history_count = 0;

// Cleanup and exit on Ctrl-C
void cleanup_exit(int sig) {
    printf("\nExiting meowshell...\n");
    for (int i = 0; i < history_count; i++) {
        printf("%s (pid=%d, ran at %s, took %lf sec)\n",
               history[i].command,
               history[i].pid,
               ctime(&history[i].start_time),
               history[i].duration);
        free(history[i].command);
    }
    exit(0);
}

// Read user command line
char *read_command_line(void) {
    char *input = NULL;
    size_t len = 0;
    int chread = getline(&input, &len, stdin);
    if (chread == -1) return NULL; 
    if (input[chread-1] == '\n') input[chread-1] = '\0'; // strip newline
    return input; // caller frees
}

// Split string into tokens
int split(char *str, char *delim, char **tokens, int max) {
    int count = 0;
    char *token = strtok(str, delim);
    while (token != NULL && count < max) {
        tokens[count++] = token;
        token = strtok(NULL, delim);
    }
    return count;
}

// Execute a pipeline of commands
void run_pipeline(char *input) {
    char *cmd[MAXCMDS];
    int ncmds = split(input, "|", cmd, MAXCMDS);

    int ppfd[2*ncmds]; // each command needs a pipe (except last)
    for (int i = 0; i < ncmds-1; i++) {
        if (pipe(ppfd + i*2) < 0) { perror("pipe"); return; }
    }

    pid_t pids[MAXCMDS];
    struct timeval start, end;
    gettimeofday(&start, NULL);
    time_t start_time = time(NULL);

    for (int i = 0; i < ncmds; i++) {
        char *argv[MAXCMDS];
        int argc = split(cmd[i], " ", argv, MAXCMDS);
        argv[argc] = NULL;

        if (argv[0] == NULL) continue;

        // cd or history or cd -
        if (ncmds == 1) {
            if (strcmp(argv[0], "cd") == 0) {
                char *newdir;
                char *oldir;
                char pwd[1024];
                if (getcwd(pwd, sizeof(pwd)) != NULL) {
                    setenv("OLDPWD", pwd, 1);
                }
                if (argv[1] == NULL) {
                    newdir = getenv("HOME");
                    oldir = getenv("OLDPWD");
                    if (newdir != NULL){
                        argv[1] = newdir;
                    }
                    else{
                        argv[1] = "/"; }
                    }
                else if (strcmp(argv[1], "-") == 0){
                    if (oldir != NULL){
                        argv[1] = oldir;
                    }
                    else{
                        printf("old directory not saved\n");
                    }
                }    
                if (chdir(argv[1]) != 0) perror("cd");
                return;
            } else if (strcmp(argv[0], "history") == 0) {
                for (int j = 0; j < history_count; j++) {
                    printf("%d %s\n", j+1, history[j].command);
                }
                return;
            }
        }
        pid_t pid = fork();
        if (pid == 0) {
            // redirect pipes
            if (i != 0) {
                dup2(ppfd[(i-1)*2], STDIN_FILENO);
            }
            if (i != ncmds-1) {
                dup2(ppfd[i*2 + 1], STDOUT_FILENO);
            }
            // close all pipe fds in child
            for (int j = 0; j < 2*(ncmds-1); j++) close(ppfd[j]);
            execvp(argv[0], argv);
            perror("execvp");
            exit(1);
        } else if (pid > 0) {
            pids[i] = pid;
        } else {
            perror("fork");
            return;
        }
    }
    // close all pipe fds in parent
    for (int i = 0; i < 2*(ncmds-1); i++) close(ppfd[i]);
    // wait for all children
    for (int i = 0; i < ncmds; i++) {
        int status;
        waitpid(pids[i], &status, 0);
    }
    gettimeofday(&end, NULL);

    // record in history
    HistoryEntry entry;
    entry.command = strdup(input);
    entry.pid = pids[ncmds-1];
    entry.start_time = start_time;
    entry.duration = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec)/1e6;
    if (history_count < MAXHISTORY) history[history_count++] = entry;
}

int main(void) {
    setenv("TZ", "Asia/Kolkata", 1);
    tzset();
    signal(SIGINT, cleanup_exit);
    while (1) {
        printf("meowshell:~$ ");
        char *input = read_command_line();
        if (!input) break; // EOF (Ctrl-D)
        if (strlen(input) == 0) { 
            free(input); continue; }
        run_pipeline(input);
        free(input);
    }
    cleanup_exit(0);
    return 0;
}
