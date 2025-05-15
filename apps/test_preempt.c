#include <stdio.h>
#include <stdlib.h>
#include <uthread.h>

int thread2() {
    printf("Entered thread 2\n");
    exit(0);  // Exit the program when thread 2 is entered
    return 0;
}

int thread1() {
    printf("Entered thread 1\n");
    uthread_create(thread2);  // Create thread 2
    printf("Created thread 2\n");

    // Thread 1 will yield control to thread 2 when preempted
    while (1) {}

    return 0;
}

int main(void) {
    uthread_start(0);  // Start the threading library
    uthread_join(uthread_create(thread1), NULL);  // Join thread 1, which creates thread 2
    uthread_stop();  // Stop the threading library when done

    return 0;
}
