#include <assert.h>
#include <signal.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include "private.h"
#include "uthread.h"
#include "queue.h"

/* Thread states */
enum {
    READY,
    RUNNING,
    BLOCKED,
    EXITED
};

struct uthread_tcb {
    uthread_ctx_t context;     /* Thread context */
    void *stack;               /* Thread stack */
    int state;                 /* Thread state */
};

/* Ready queue and pointer to currently running thread */
static queue_t ready_queue;
static struct uthread_tcb *running_thread;
static struct uthread_tcb *idle_thread;

struct uthread_tcb *uthread_current(void)
{
    return running_thread;
}

static void uthread_schedule(void)
{
    struct uthread_tcb *prev_thread = running_thread;
    struct uthread_tcb *next_thread;

    /* Get next thread from ready queue */
    if (queue_dequeue(ready_queue, (void**)&next_thread) < 0) {
        /* No more threads to run, back to idle thread */
        next_thread = idle_thread;
    }

    /* Update thread states */
    running_thread = next_thread;
    running_thread->state = RUNNING;

    /* Switch context to next thread */
    uthread_ctx_switch(&prev_thread->context, &next_thread->context);
}

void uthread_yield(void)
{
    /* Disable preemption during scheduling operation */
    preempt_disable();

    /* Put current thread back in ready queue if not idle and not exited */
    if (running_thread != idle_thread && running_thread->state != EXITED) {
        running_thread->state = READY;
        queue_enqueue(ready_queue, running_thread);
    }

    /* Schedule next thread */
    uthread_schedule();

    /* Re-enable preemption */
    preempt_enable();
}

void uthread_exit(void)
{
    /* Disable preemption during exit */
    preempt_disable();

    /* Mark thread as exited */
    running_thread->state = EXITED;

    /* Free thread resources - the stack will be freed here */
    if (running_thread != idle_thread) {
        uthread_ctx_destroy_stack(running_thread->stack);
        /* We don't free the TCB yet since we're still using its context */
    }

    /* Schedule next thread */
    uthread_schedule();

    /* This point should never be reached */
    fprintf(stderr, "Error: uthread_exit reached end\n");
    exit(1);
}

int uthread_create(uthread_func_t func, void *arg)
{
    struct uthread_tcb *tcb;

    /* Disable preemption during thread creation */
    preempt_disable();

    /* Allocate TCB */
    tcb = malloc(sizeof(struct uthread_tcb));
    if (tcb == NULL) {
        preempt_enable();
        return -1;
    }

    /* Allocate stack */
    tcb->stack = uthread_ctx_alloc_stack();
    if (tcb->stack == NULL) {
        free(tcb);
        preempt_enable();
        return -1;
    }

    /* Initialize context */
    if (uthread_ctx_init(&tcb->context, tcb->stack, func, arg) < 0) {
        uthread_ctx_destroy_stack(tcb->stack);
        free(tcb);
        preempt_enable();
        return -1;
    }

    /* Set thread as ready and add to ready queue */
    tcb->state = READY;
    if (queue_enqueue(ready_queue, tcb) < 0) {
        uthread_ctx_destroy_stack(tcb->stack);
        free(tcb);
        preempt_enable();
        return -1;
    }

    /* Re-enable preemption */
    preempt_enable();
    return 0;
}

int uthread_run(bool preempt, uthread_func_t func, void *arg)
{
    /* Create ready queue */
    ready_queue = queue_create();
    if (ready_queue == NULL)
        return -1;

    /* Create idle thread (main thread becomes idle thread) */
    idle_thread = malloc(sizeof(struct uthread_tcb));
    if (idle_thread == NULL) {
        queue_destroy(ready_queue);
        return -1;
    }

    /* Initialize idle thread */
    idle_thread->state = RUNNING;
    running_thread = idle_thread;

    /* Start preemption if requested */
    preempt_start(preempt);

    /* Create initial thread */
    if (uthread_create(func, arg) < 0) {
        queue_destroy(ready_queue);
        free(idle_thread);
        preempt_stop();
        return -1;
    }

    /* Run scheduling loop until all threads complete */
    while (queue_length(ready_queue) > 0)
        uthread_yield();

    /* Cleanup */
    free(idle_thread);
    queue_destroy(ready_queue);
    preempt_stop();

    return 0;
}

void uthread_block(void)
{
    /* Disable preemption */
    preempt_disable();

    /* Mark thread as blocked */
    running_thread->state = BLOCKED;

    /* Schedule next thread */
    uthread_schedule();

    /* Re-enable preemption */
    preempt_enable();
}

void uthread_unblock(struct uthread_tcb *uthread)
{
    /* Sanity check */
    if (uthread == NULL)
        return;

    /* Disable preemption */
    preempt_disable();

    /* Only unblock if thread is in BLOCKED state */
    if (uthread->state == BLOCKED) {
        uthread->state = READY;
        queue_enqueue(ready_queue, uthread);
    }

    /* Re-enable preemption */
    preempt_enable();
}