#include <stddef.h>
#include <stdlib.h>

#include "queue.h"
#include "private.h"
#include "sem.h"

struct semaphore {
    size_t count;         /* Current value of semaphore */
    queue_t waiting_queue; /* Queue of threads waiting on this semaphore */
};

sem_t sem_create(size_t count)
{
    sem_t sem = malloc(sizeof(struct semaphore));
    if (sem == NULL)
        return NULL;
    
    sem->count = count;
    sem->waiting_queue = queue_create();
    if (sem->waiting_queue == NULL) {
        free(sem);
        return NULL;
    }
    
    return sem;
}

int sem_destroy(sem_t sem)
{
    if (sem == NULL)
        return -1;
    
    /* Check if there are still threads waiting on this semaphore */
    if (queue_length(sem->waiting_queue) > 0)
        return -1;
    
    queue_destroy(sem->waiting_queue);
    free(sem);
    return 0;
}

int sem_down(sem_t sem)
{
    if (sem == NULL)
        return -1;
    
    /* Disable preemption to ensure atomicity */
    preempt_disable();
    
    /* If resource is available, take it */
    if (sem->count > 0) {
        sem->count--;
        preempt_enable();
        return 0;
    }
    
    /* Resource not available, block the current thread */
    struct uthread_tcb *current = uthread_current();
    
    /* Add thread to waiting queue */
    queue_enqueue(sem->waiting_queue, current);
    
    /* Block thread - this will re-enable preemption */
    uthread_block();
    
    /* When we get here, we've been unblocked, but need to recheck the resource
     * to handle the corner case described in the spec */
    preempt_disable();
    
    /* If count is 0, someone else got the resource while we were waiting to run.
     * Put ourselves back in the waiting queue at the front to prevent starvation */
    if (sem->count == 0) {
        /* Create temporary queue to preserve ordering - add current thread first */
        queue_t temp_queue = queue_create();
        queue_enqueue(temp_queue, current);
        
        /* Move all other waiting threads after current thread */
        struct uthread_tcb *thread;
        while (queue_dequeue(sem->waiting_queue, (void**)&thread) == 0) {
            queue_enqueue(temp_queue, thread);
        }
        
        /* Swap the queues and destroy temporary queue */
        queue_t old_queue = sem->waiting_queue;
        sem->waiting_queue = temp_queue;
        queue_destroy(old_queue);
        
        /* Block again */
        preempt_enable();
        uthread_block();
        
        /* When we get here again, the count should be > 0 */
        preempt_disable();
    }
    
    /* Take the resource */
    sem->count--;
    preempt_enable();
    return 0;
}

int sem_up(sem_t sem)
{
    if (sem == NULL)
        return -1;
    
    /* Disable preemption to ensure atomicity */
    preempt_disable();
    
    /* Increment resource count */
    sem->count++;
    
    /* If threads are waiting, unblock the first one */
    struct uthread_tcb *thread;
    if (queue_dequeue(sem->waiting_queue, (void**)&thread) == 0) {
        uthread_unblock(thread);
    }
    
    preempt_enable();
    return 0;
}