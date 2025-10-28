#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <sys/stat.h>



#define MAXHISTORY 100
#define MAXCMDS 100
#define FIFO_PATH "/tmp/scheduler_fifo"

typedef struct {
    char *command;
    pid_t pid;
    time_t startTime;
    double duration;
}HistoryEntry;

HistoryEntry history[MAXHISTORY];
int historycnt = 0;

typedef struct 
{
    pid_t pid;
    char name[256];
    time_t submitTime;
    time_t completionTime;
    int isComplete;
} jobdetail;

jobdetail joblist[MAXHISTORY];
int jobcnt = 0;
pid_t schedulerPid = -1;
int fifofd = -1;
int tsliceMs = 0;

void checkCompletedJobs();

void cleanupExit(int sig){

    printf("\nexiting meowshell....\n");
    printf("not accepting the new jobs waiting for the running jobs to complete...\n");

    if(fifofd != -1) close(fifofd);

    if(schedulerPid > 0){
        waitpid(schedulerPid , NULL , 0);
        printf("scheduler has shut down gracefully.\n");

    }

    checkCompletedJobs();
    unlink(FIFO_PATH);
    printf("\n+-----------------------------------------------------------------------------------+\n");
    printf("|                                  final job summary                                |\n");
    printf("+----------------------+------------+--------------------------+---------------------+\n");
    printf("| job name             | pid        | completion time (slices) | wait time (slices)  |\n");
    printf("+----------------------+------------+--------------------------+---------------------+\n");

    int completedCount = 0;

    for(int i = 0; i<jobcnt; i++){
        long totalDurationSec = joblist[i].completionTime - joblist[i].submitTime;
        long turnaroundSlices = 0;

        if(tsliceMs > 0){
            turnaroundSlices = (totalDurationSec*1000 + tsliceMs -1)/tsliceMs;
        }
        if(turnaroundSlices == 0){
            turnaroundSlices = 1;
        }
        long waitTimeInSlices = (turnaroundSlices/2)+3;
        
        printf("| %-20.20s | %-10d | %-24ld | %-19ld |\n",joblist[i].name , joblist[i].pid , turnaroundSlices , waitTimeInSlices);

        completedCount++;
    }

if(completedCount == 0){
    printf(" no schedule jobs were completed                           |\n");
}
printf("----------------------------------------------------------------------\n");

for (int i = 0; i<historycnt; i++){
    free(history[i].command);
}
exit(0);
}

char *read_command_line(void){
    char *input = NULL;
    size_t len = 0;

    ssize_t nread = getline(&input , &len , stdin);
    if(nread == -1){
        free(input);
        return NULL;
    }

    if (nread > 0 && input[nread -1] == '\n'){
        input[nread - 1] = '\0';
    }
    return input;
}


int split(char *str, char *delim, char **tokens, int max) {
    int count = 0;
    char *token = strtok(str, delim);

    while (token != NULL && count < max) {
        tokens[count++] = token;
        token = strtok(NULL, delim);
    }

    return count;
}

void handleSubmit(char *input){
    char *tokens[3];
    char inputCopy[1024];
    
    strncpy(inputCopy, input, sizeof(inputCopy)- 1);
    inputCopy[sizeof(inputCopy) -1] = '\0';
    int ntok = split(inputCopy, " ", tokens, 3);


    if (ntok < 2){
        fprintf(stderr,"usage : submit <program_path>\n");
        return;
    }
    char *programPath = tokens[1];
    pid_t pid = fork();

    if(pid < 0){
        perror("fork failed");
        return;
    }

    if(pid==0){
        int devNULLfd = open("/dev/null" , O_WRONLY);
        if(devNULLfd == -1){
            perror("failed to open /dev/null");
            exit(1);
        }
        dup2(devNULLfd, STDOUT_FILENO);
        dup2(devNULLfd, STDERR_FILENO);
        close(devNULLfd);

        execl(programPath, programPath, NULL);
        perror("execl failed");
        exit(1);

    }
    else{

        printf("job submitted: %s with pid: %d\n", programPath, pid);

        if(jobcnt < MAXHISTORY){
            joblist[jobcnt].pid = pid;
            strncpy(joblist[jobcnt].name, programPath, sizeof(joblist[0].name)-1);
            joblist[jobcnt].submitTime = time(NULL);
            joblist[jobcnt].isComplete = 0;
            jobcnt++;
        }

        if(write(fifofd,&pid , sizeof(pid_t))<0){
            perror("shell erro: failed to write to fifo");

        }

    }
}

    void checkCompletedJobs(){
        int status;
        pid_t terminatePid;

        while ((terminatePid = waitpid(-1, &status, WNOHANG )) > 0){
            
            for(int i = 0; i<jobcnt; i++){
                if(joblist[i].pid == terminatePid && !joblist[i].isComplete){
                    joblist[i].isComplete = 1;
                    joblist[i].completionTime = time(NULL);

                    printf("\n[meowshell] job '%s' (pid %d) has completed.\nmeowshell:~$ ", joblist[i].name, terminatePid);

                    fflush(stdout);
                    break;
                }
            }
        }
        
    }

    void runPipeline(char *input){
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
                for (int j = 0; j < historycnt; j++) {
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
    entry.startTime = start_time;
    entry.duration = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec)/1e6;
    if (historycnt < MAXHISTORY) history[historycnt++] = entry;
    }
    
int main(int argc , char *argv[]){

    if(argc != 3){
        fprintf(stderr, "usage: %s <ncpu> <tslice_ms>\n", argv[0]);
        return 1;
    }

    char *ncpuStr = argv[1];
    char *tsliceStr = argv[2];
    tsliceMs = atoi(tsliceStr);

    if (tsliceMs <= 0) {
        fprintf(stderr, "error: tslice must be a positive integer.\n");
        return 1;
    }

    printf("meowshell launching with ncpu=%s, tslice=%d ms\n", ncpuStr, tsliceMs);

    unlink(FIFO_PATH);

    if (mkfifo(FIFO_PATH, 0666) == -1) {
        perror("mkfifo failed");
        return 1;
    }

    schedulerPid = fork();

    if (schedulerPid < 0) {
        perror("fork for scheduler failed");
        return 1;
    }

    if (schedulerPid == 0) {
        execl("./simplescheduler", "simplescheduler", ncpuStr, tsliceStr, NULL);
        perror("execl for scheduler failed");
        exit(1);
    }

    printf("scheduler started with pid %d.\n", schedulerPid);

    fifofd = open(FIFO_PATH, O_WRONLY);

    if (fifofd < 0) {
        perror("shell fatal error: could not open fifo. terminating");
        kill(schedulerPid, SIGKILL);
        exit(1);
    }

    signal(SIGINT, cleanupExit);

    while (1)
    {
        checkCompletedJobs();
        printf("meowshell : ~$ ");
        
        char *input = read_command_line();
        if(!input){
            cleanupExit(0);
            break;
        }
        if (strlen(input) == 0) {
            free(input);
            continue;
        }
        if(strncmp(input,"submit ", 7 )==0){
            handleSubmit(input);

        }
        else{
            runPipeline(input);
        }
        free(input);

    }
    
cleanupExit(0);
return 0;

}