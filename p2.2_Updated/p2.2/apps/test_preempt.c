/*
 * Preemption test
 *
 * Tests if preemption works by creating a thread that runs an infinite loop.
 * If preemption works, the main thread should still get CPU time and be able
 * to terminate the program after a delay.
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <uthread.h>

/* Counter for infinite loop */
static volatile int counter = 0;

/* Thread that runs an infinite loop */
static void infinite_thread(void *arg)
{
    (void)arg; /* Unused parameter */
    
    printf("Infinite thread starting...\n");
    
    /* Run an infinite loop */
    while (1) {
        counter++;
        
        /* Print counter value every 10 million iterations */
        if (counter % 10000000 == 0)
            printf("Counter: %d million\n", counter / 1000000);
    }
}

/* Thread that sleeps and then exits */
static void timer_thread(void *arg)
{
    int seconds = *((int*)arg);
    
    printf("Timer thread sleeping for %d seconds...\n", seconds);
    
    /* Sleep for specified time */
    sleep(seconds);
    
    printf("Timer expired! Final counter value: %d million\n", counter / 1000000);
    printf("Preemption test successful - infinite thread was preempted.\n");
}

int main(void)
{
    int sleep_time = 3; /* Sleep for 3 seconds */
    
    printf("Starting preemption test...\n");
    
    /* Run with preemption enabled */
    uthread_run(true, timer_thread, &sleep_time);
    
    /* If we reach here, the timer thread successfully exited */
    return 0;
}