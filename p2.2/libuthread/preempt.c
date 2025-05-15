#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/time.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>
#include "private.h"
#include "preempt.h"

/* Global variables to hold the previous signal handler and the timer settings */
static struct sigaction old_sigaction;
static struct itimerval old_timer;

/* Signal handler for SIGVTALRM (used for preemption) */
void preempt_handler(int sig) {
    uthread_yield();  // Yield the current thread
}

/* Function to start preemption */
void preempt_start() {
    struct sigaction sa;

    // Set up the signal handler
    memset(&sa, 0, sizeof(struct sigaction));
    sa.sa_handler = preempt_handler;
    sa.sa_flags = SA_RESTART;  // Restart system calls if interrupted

    // Save the current signal action for SIGVTALRM
    sigaction(SIGVTALRM, NULL, &old_sigaction);

    // Install the new signal handler
    sigaction(SIGVTALRM, &sa, NULL);

    // Set up the timer to trigger SIGVTALRM 100 times per second
    struct itimerval timer;
    timer.it_value.tv_sec = 0;
    timer.it_value.tv_usec = 10000;  // 10 milliseconds = 100 Hz
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = 10000;

    // Set the timer to send SIGVTALRM
    setitimer(ITIMER_VIRTUAL, &timer, &old_timer);
}

/* Function to stop preemption */
void preempt_stop() {
    // Restore the original signal handler
    sigaction(SIGVTALRM, &old_sigaction, NULL);

    // Restore the original timer settings
    setitimer(ITIMER_VIRTUAL, &old_timer, NULL);
}

/* Function to enable preemption by blocking the SIGVTALRM signal */
void preempt_enable() {
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGVTALRM);
    sigprocmask(SIG_UNBLOCK, &mask, NULL);  // Unblock the SIGVTALRM signal
}

/* Function to disable preemption by blocking the SIGVTALRM signal */
void preempt_disable() {
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGVTALRM);
    sigprocmask(SIG_BLOCK, &mask, NULL);  // Block the SIGVTALRM signal
}
