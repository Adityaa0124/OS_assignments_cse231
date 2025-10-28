// dummy_main.h
#ifndef DUMMY_MAIN_H
#define DUMMY_MAIN_H

#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <time.h>

int dummy_main(int argc, char **argv) {
    printf("Dummy job started: PID=%d\n", getpid());

    // Example: run for 5 seconds
    for (int i = 0; i < 5; i++) {
        printf("PID %d: working %d/5\n", getpid(), i+1);
        sleep(1);
    }

    printf("Dummy job finished: PID=%d\n", getpid());
    return 0;
}

#endif
