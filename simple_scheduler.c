#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <errno.h>

#define FIFO_PATH "/tmp/scheduler_fifo"
#define SHM_PATH "/simplesched_shm"
#define MAX_JOBS 100

typedef struct {
    pid_t pid;
    int arrivalSlice;
    int startSlice;
    int finishSlice;
    int runSlices;
    int isComplete;
    char name[128];
} JobInfo;

pid_t readyQueue[MAX_JOBS];
int head = 0, tail = 0, size = 0;
pid_t running[MAX_JOBS];
int runCount = 0;
int ncpu, tslice;
int currentSlice = 0;
JobInfo *jobs;

void enqueue(pid_t pid) {
    if (size < MAX_JOBS) {
        readyQueue[tail] = pid;
        tail = (tail + 1) % MAX_JOBS;
        size++;
    }
}

pid_t dequeue() {
    if (size == 0) return -1;
    pid_t pid = readyQueue[head];
    head = (head + 1) % MAX_JOBS;
    size--;
    return pid;
}

int findJob(pid_t pid) {
    for (int i = 0; i < MAX_JOBS; i++)
        if (jobs[i].pid == pid) return i;
    return -1;
}

int isTerminated(pid_t pid) {
    return (kill(pid, 0) == -1 && errno == ESRCH);
}

void handleShutdown(int sig) {
    printf("\n[scheduler] shutting down...\n");
    munmap(jobs, sizeof(JobInfo) * MAX_JOBS);
    shm_unlink(SHM_PATH);
    exit(0);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "usage: %s <ncpu> <tslice_ms>\n", argv[0]);
        return 1;
    }

    ncpu = atoi(argv[1]);
    tslice = atoi(argv[2]);
    if (ncpu <= 0 || tslice <= 0) return 1;

    int shmfd = shm_open(SHM_PATH, O_CREAT | O_RDWR, 0666);
    ftruncate(shmfd, sizeof(JobInfo) * MAX_JOBS);
    jobs = mmap(0, sizeof(JobInfo) * MAX_JOBS, PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);

    signal(SIGINT, handleShutdown);
    signal(SIGTERM, handleShutdown);

    int fifo = open(FIFO_PATH, O_RDONLY | O_NONBLOCK);
    if (fifo < 0) { perror("fifo open"); return 1; }

    printf("[scheduler] started with ncpu=%d, tslice=%dms\n", ncpu, tslice);

    while (1) {
        pid_t newpid;
        ssize_t bytes = read(fifo, &newpid, sizeof(pid_t));
        if (bytes > 0) {
            printf("[scheduler] got new job pid=%d\n", newpid);
            int idx = findJob(-1);
            if (idx == -1) {
                for (int i = 0; i < MAX_JOBS; i++)
                    if (jobs[i].pid == 0) { idx = i; break; }
            }
            if (idx != -1) {
                jobs[idx].pid = newpid;
                jobs[idx].arrivalSlice = currentSlice;
                jobs[idx].isComplete = 0;
                enqueue(newpid);
            }
        }

        // Stop running jobs
        for (int i = 0; i < runCount; i++) {
            if (!isTerminated(running[i])) kill(running[i], SIGSTOP);
        }

        // Remove terminated jobs
        int newCount = 0;
        for (int i = 0; i < runCount; i++) {
            if (isTerminated(running[i])) {
                int idx = findJob(running[i]);
                if (idx != -1) {
                    jobs[idx].finishSlice = currentSlice;
                    jobs[idx].isComplete = 1;
                }
            } else {
                enqueue(running[i]);
            }
        }

        runCount = 0;

        // Start next jobs
        for (int i = 0; i < ncpu && size > 0; i++) {
            pid_t pid = dequeue();
            if (isTerminated(pid)) continue;
            int idx = findJob(pid);
            if (idx == -1) continue;
            if (jobs[idx].startSlice == 0) jobs[idx].startSlice = currentSlice;
            jobs[idx].runSlices++;
            kill(pid, SIGCONT);
            running[runCount++] = pid;
        }

        currentSlice++;
        usleep(tslice * 1000);

        // Exit if all done
        int allDone = 1;
        for (int i = 0; i < MAX_JOBS; i++) {
            if (jobs[i].pid && !jobs[i].isComplete) { allDone = 0; break; }
        }
        if (allDone && size == 0) break;
    }

    handleShutdown(0);
    return 0;
}
