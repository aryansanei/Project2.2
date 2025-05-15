#include <signal.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include "private.h"
#include "uthread.h"

/*
 * Frequency of preemption
 * 100Hz is 100 times per second
 */
#define HZ 100

/* Signal action for SIGVTALRM */
static struct sigaction sa_old;

/* Timer configuration */
static struct itimerval timer_old;

/* Flag indicating if preemption is enabled */
static bool preemption_enabled = false;

/* Signal mask to block/unblock SIGVTALRM */
static sigset_t vtalrm_mask;

/* Signal handler for SIGVTALRM */
static void timer_handler(int signo)
{
    (void)signo; /* Unused parameter */
    
    /* Force the currently running thread to yield */
    uthread_yield();
}

void preempt_disable(void)
{
    /* Only block signals if preemption is enabled */
    if (preemption_enabled)
        sigprocmask(SIG_BLOCK, &vtalrm_mask, NULL);
}

void preempt_enable(void)
{
    /* Only unblock signals if preemption is enabled */
    if (preemption_enabled)
        sigprocmask(SIG_UNBLOCK, &vtalrm_mask, NULL);
}

void preempt_start(bool preempt)
{
    if (!preempt)
        return;
    
    /* Initialize mask to block SIGVTALRM */
    sigemptyset(&vtalrm_mask);
    sigaddset(&vtalrm_mask, SIGVTALRM);
    
    /* Install signal handler */
    struct sigaction sa;
    sa.sa_handler = timer_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    
    if (sigaction(SIGVTALRM, &sa, &sa_old) < 0) {
        perror("sigaction");
        exit(1);
    }
    
    /* Configure timer */
    struct itimerval timer;
    
    /* Set interval between timer events */
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = 1000000 / HZ; /* 10ms for 100Hz */
    
    /* Set initial expiration */
    timer.it_value = timer.it_interval;
    
    if (setitimer(ITIMER_VIRTUAL, &timer, &timer_old) < 0) {
        perror("setitimer");
        exit(1);
    }
    
    /* Mark preemption as enabled */
    preemption_enabled = true;
}

void preempt_stop(void)
{
    /* Only restore if preemption was enabled */
    if (!preemption_enabled)
        return;
    
    /* Restore old timer configuration */
    if (setitimer(ITIMER_VIRTUAL, &timer_old, NULL) < 0) {
        perror("setitimer");
        exit(1);
    }
    
    /* Restore old signal action */
    if (sigaction(SIGVTALRM, &sa_old, NULL) < 0) {
        perror("sigaction");
        exit(1);
    }
    
    /* Mark preemption as disabled */
    preemption_enabled = false;
}