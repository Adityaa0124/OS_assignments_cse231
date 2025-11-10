#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>

#define FIFO_PATH "/tmp/scheduler_fifo"
#define MAX_JOBS 100


pid_t readyQueue[MAX_JOBS];
int queueHead = 0, queueTail = 0, queueSize = 0;

pid_t runningPids[MAX_JOBS];
int runningCount = 0;

int ncpu = 1;

void enqueue(pid_t pid) {
    if (queueSize < MAX_JOBS) {
        readyQueue[queueTail] = pid;
        queueTail = (queueTail + 1) % MAX_JOBS;
        queueSize++;
    }
     else{
        fprintf(stderr, "[scheduler] ready queue full! pid %d dropped.\n", pid);
    }
}

pid_t dequeue() {
    if (queueSize > 0) {
        pid_t pid = readyQueue[queueHead];
        queueHead = (queueHead + 1) % MAX_JOBS;
        queueSize--;
        return pid;
    }
    return -1;
}

void handleForcedShutdown(int sig) {
    printf("\n[scheduler] forced shutdown. killing running jobs.\n");
    for (int i = 0; i < runningCount; i++) {
        kill(runningPids[i], SIGKILL);
    }
    exit(1);
}

int isTerminated(pid_t pid) {
    return (kill(pid, 0) == -1 && errno == ESRCH);
}

int main(int argc, char *argv[]) {

    if (argc != 3) {
        fprintf(stderr, "usage: %s <ncpu> <tslice_ms>\n", argv[0]);
        return 1;
    }

    ncpu = atoi(argv[1]);
    int tsliceMs = atoi(argv[2]);

    if (ncpu <= 0 || tsliceMs <= 0) {
        fprintf(stderr, "[scheduler] ncpu and tslice must be positive.\n");
        return 1;
    }

    printf("[scheduler] started with ncpu=%d, tslice=%dms\n", ncpu, tsliceMs);
    signal(SIGTERM, handleForcedShutdown);
    signal(SIGINT, handleForcedShutdown);

    int fifoFd = open(FIFO_PATH, O_RDONLY | O_NONBLOCK);
    if (fifoFd == -1) {
        perror("[scheduler] failed to open fifo");
        return 1;
    }

    int shellActive = 1;
    while (shellActive || queueSize > 0 || runningCount > 0) {
      
        if (shellActive) {
            pid_t newPid;
            ssize_t bytes = read(fifoFd, &newPid, sizeof(pid_t));

            if (bytes > 0) {
                printf("[scheduler] received new job pid: %d\n", newPid);
                enqueue(newPid);
            } 
            else if (bytes == 0) {
                shellActive = 0;
                close(fifoFd);
            }
        }
        int aliveCount = 0;
        pid_t alivePids[MAX_JOBS];
        for (int i = 0; i < runningCount; i++) {
            pid_t pid = runningPids[i];
            if (isTerminated(pid)) {
                printf("[scheduler] job %d finished.\n", pid);
            } else {
                kill(pid, SIGSTOP);
                alivePids[aliveCount++] = pid;
            }
        }

        runningCount = 0;
        for (int i = 0; i < aliveCount; i++) {
            enqueue(alivePids[i]);
        }

        while (runningCount < ncpu && queueSize > 0) {
            pid_t pidToRun = dequeue();

            if (!isTerminated(pidToRun) && kill(pidToRun, SIGCONT) == 0) {
                runningPids[runningCount++] = pidToRun;
            } else {
                if (isTerminated(pidToRun)) {
                    printf("[scheduler] job %d terminated before running.\n", pidToRun);
                } else {
                    perror("[scheduler] error continuing process");
                }
            }
        }
        usleep(tsliceMs*1000);
    }

    printf("[scheduler] all jobs completed. exiting gracefully.\n");
    return 0;
}
