#include "dummy_main.h"
#include <stdio.h>
#include <unistd.h>

int main() {
    char *messages[] = {"Hello", "World", "from", "Scheduler", "Test"};
    for (int i = 0; i < 5; i++) {
        printf("[printer_job] PID %d: %s\n", getpid(), messages[i]);
        sleep(2);
    }
    return 0;
}
