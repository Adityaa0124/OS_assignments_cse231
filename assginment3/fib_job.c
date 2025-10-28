#include "dummy_main.h"
#include <stdio.h>
#include <unistd.h>

int main() {
    int n = 10, t1 = 0, t2 = 1;
    printf("[fib_job] PID %d: Fibonacci series: ", getpid());
    for (int i = 1; i <= n; i++) {
        printf("%d ", t1);
        int next = t1 + t2;
        t1 = t2;
        t2 = next;
        sleep(1);  // Simulate work
    }
    printf("\n");
    return 0;
}
