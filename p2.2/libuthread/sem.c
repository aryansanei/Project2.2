#include <stddef.h>
#include <stdlib.h>

#include "queue.h"
#include "private.h"
#include "sem.h"


struct semaphore {
    size_t count;             // The count of available resources
    queue_t waiting_threads;  // A queue of threads waiting for the semaphore
};


sem_t sem_create(size_t count) {
    // Allocate memory for the semaphore
    sem_t sem = (sem_t)malloc(sizeof(struct semaphore));
    if (sem == NULL) {
        return NULL;  // Memory allocation failed
    }

    // Initialize semaphore fields
    sem->count = count;
    sem->waiting_threads = queue_create();  // Assuming you have a queue API

    return sem;
}


int sem_destroy(sem_t sem) {
    if (sem == NULL || !queue_empty(sem->waiting_threads)) {
        return -1;  // Semaphore is NULL or there are waiting threads
    }

    // Free the waiting threads queue
    queue_destroy(sem->waiting_threads);
    // Free the semaphore structure
    free(sem);

    return 0;
}


int sem_down(sem_t sem) {
    if (sem == NULL) {
        return -1;  // Semaphore is NULL
    }

    // Decrease the semaphore count
    sem->count--;

    // If the count is less than 0, add the current thread to the waiting queue
    if (sem->count < 0) {
        thread_block(sem->waiting_threads);  // Block the current thread
    }

    return 0;
}


int sem_up(sem_t sem) {
    if (sem == NULL) {
        return -1;  // Semaphore is NULL
    }

    // Increment the semaphore count
    sem->count++;

    // If there are threads waiting, unblock the first one
    if (sem->count <= 0) {
        thread_unblock(sem->waiting_threads);  // Unblock the oldest thread
    }

    return 0;
}
