// simpleshell.c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <errno.h>

#define MAX_JOBS 100
#define FIFO_PATH "/tmp/scheduler_fifo"
#define SHM_PATH "/simplesched_shm"

typedef struct {
    pid_t pid;
    int arrivalSlice;
    int startSlice;
    int finishSlice;
    int runSlices;
    int isComplete;
    char name[128];
} JobInfo;

int schedulerPid = -1;
int fifoFd = -1;
int tsliceMs = 0;
JobInfo *jobs;

// --------------------------- Utility Functions --------------------------- //

void cleanup() {
    if (fifoFd != -1) close(fifoFd);
    if (schedulerPid > 0) {
        kill(schedulerPid, SIGTERM);
        waitpid(schedulerPid, NULL, 0);
    }
    shm_unlink(SHM_PATH);
    unlink(FIFO_PATH);
}

// ------------------------------------------------------------------------ //
// Print the final summary table using data from shared memory
// ------------------------------------------------------------------------ //
void printJobSummary() {
    printf("\n+-----------------------------------------------------------------------------------+\n");
    printf("|                                  final job summary                                |\n");
    printf("+----------------------+------------+--------------------------+---------------------+\n");
    printf("| job name             | pid        | completion time (slices) | wait time (slices)  |\n");
    printf("+----------------------+------------+--------------------------+---------------------+\n");

    for (int i = 0; i < MAX_JOBS; i++) {
        if (jobs[i].pid != 0 && jobs[i].isComplete) {
            int turnaround = jobs[i].finishSlice - jobs[i].arrivalSlice + 1;
            int wait = turnaround - jobs[i].runSlices;
            if (wait < 0) wait = 0;
            printf("| %-20.20s | %-10d | %-24d | %-19d |\n",
                   jobs[i].name, jobs[i].pid, turnaround, wait);
        }
    }
    printf("+-----------------------------------------------------------------------------------+\n");
}

// ------------------------------------------------------------------------ //
// Called when user presses Ctrl+C or shell exits
// ------------------------------------------------------------------------ //
void handleExit(int sig) {
    printf("\n[simpleshell] Exiting... waiting for scheduler and jobs to finish...\n");
    sleep(1);
    printJobSummary();
    cleanup();
    printf("[simpleshell] Shutdown complete.\n");
    exit(0);
}

// ------------------------------------------------------------------------ //
// Reads a full line from stdin safely
// ------------------------------------------------------------------------ //
char *readCommand() {
    char *line = NULL;
    size_t len = 0;
    ssize_t read = getline(&line, &len, stdin);
    if (read == -1) {
        free(line);
        return NULL;
    }
    if (read > 0 && line[read - 1] == '\n')
        line[read - 1] = '\0';
    return line;
}

// ------------------------------------------------------------------------ //
// Split command line by space into tokens
// ------------------------------------------------------------------------ //
int split(char *str, char *delim, char **tokens, int max) {
    int count = 0;
    char *token = strtok(str, delim);
    while (token != NULL && count < max) {
        tokens[count++] = token;
        token = strtok(NULL, delim);
    }
    return count;
}

// ------------------------------------------------------------------------ //
// Submit a new job (fork + write PID to FIFO + record in shared memory)
// ------------------------------------------------------------------------ //
void handleSubmit(char *cmd) {
    char *tokens[3];
    char temp[512];
    strncpy(temp, cmd, sizeof(temp) - 1);
    temp[sizeof(temp) - 1] = '\0';
    int n = split(temp, " ", tokens, 3);
    if (n < 2) {
        fprintf(stderr, "Usage: submit <program_path>\n");
        return;
    }

    char *path = tokens[1];
    pid_t pid = fork();

    if (pid < 0) {
        perror("[simpleshell] fork failed");
        return;
    }

    if (pid == 0) {
        // Child process
        raise(SIGSTOP);  // Wait until scheduler SIGCONT
        execl(path, path, NULL);
        perror("[simpleshell] execl failed");
        exit(1);
    }

    // Parent
    printf("[simpleshell] Submitted job %s (pid %d)\n", path, pid);

    // Add entry in shared memory
    for (int i = 0; i < MAX_JOBS; i++) {
        if (jobs[i].pid == 0) {
            jobs[i].pid = pid;
            strncpy(jobs[i].name, path, sizeof(jobs[i].name) - 1);
            jobs[i].arrivalSlice = 0;
            jobs[i].startSlice = 0;
            jobs[i].finishSlice = 0;
            jobs[i].runSlices = 0;
            jobs[i].isComplete = 0;
            break;
        }
    }

    // Notify scheduler
    if (write(fifoFd, &pid, sizeof(pid_t)) < 0)
        perror("[simpleshell] failed to write to FIFO");
}

// ------------------------------------------------------------------------ //
// Simple interactive shell loop
// ------------------------------------------------------------------------ //
void shellLoop() {
    while (1) {
        printf("SimpleShell$ ");
        fflush(stdout);
        char *input = readCommand();
        if (!input) break;

        if (strncmp(input, "submit ", 7) == 0) {
            handleSubmit(input);
        } else if (strcmp(input, "exit") == 0 || strcmp(input, "quit") == 0) {
            free(input);
            handleExit(0);
        } else {
            printf("[simpleshell] Unknown command. Use: submit <program> or Ctrl+C to exit.\n");
        }

        free(input);
    }
}

// ------------------------------------------------------------------------ //
// Main entry
// ------------------------------------------------------------------------ //
int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <ncpu> <tslice_ms>\n", argv[0]);
        return 1;
    }

    int ncpu = atoi(argv[1]);
    tsliceMs = atoi(argv[2]);
    if (ncpu <= 0 || tsliceMs <= 0) {
        fprintf(stderr, "Error: ncpu and tslice_ms must be > 0\n");
        return 1;
    }

    printf("[simpleshell] Launching with NCPU=%d, TSLICE=%dms\n", ncpu, tsliceMs);

    signal(SIGINT, handleExit);

    unlink(FIFO_PATH);
    if (mkfifo(FIFO_PATH, 0666) < 0) {
        perror("mkfifo failed");
        return 1;
    }

    int shmfd = shm_open(SHM_PATH, O_CREAT | O_RDWR, 0666);
    if (shmfd < 0) {
        perror("shm_open failed");
        return 1;
    }
    ftruncate(shmfd, sizeof(JobInfo) * MAX_JOBS);
    jobs = mmap(0, sizeof(JobInfo) * MAX_JOBS, PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);
    memset(jobs, 0, sizeof(JobInfo) * MAX_JOBS);

    schedulerPid = fork();
    if (schedulerPid < 0) {
        perror("[simpleshell] scheduler fork failed");
        cleanup();
        return 1;
    }

    if (schedulerPid == 0) {
        execl("./simplescheduler", "simplescheduler", argv[1], argv[2], NULL);
        perror("[simpleshell] execl scheduler failed");
        exit(1);
    }

    printf("[simpleshell] Scheduler started with pid=%d\n", schedulerPid);

    fifoFd = open(FIFO_PATH, O_WRONLY);
    if (fifoFd < 0) {
        perror("FIFO open failed");
        cleanup();
        return 1;
    }

    shellLoop();
    handleExit(0);
    return 0;
}
