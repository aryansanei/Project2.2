#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"

/* Queue node structure */
struct queue_node {
    void *data;
    struct queue_node *next;
};

/* Queue structure */
struct queue {
    struct queue_node *head;  /* Oldest element */
    struct queue_node *tail;  /* Newest element */
    int length;               /* Number of elements in queue */
};

queue_t queue_create(void)
{
    queue_t queue = malloc(sizeof(struct queue));
    if (queue == NULL)
        return NULL;
    
    queue->head = NULL;
    queue->tail = NULL;
    queue->length = 0;
    
    return queue;
}

int queue_destroy(queue_t queue)
{
    if (queue == NULL)
        return -1;
    
    if (queue->length > 0)
        return -1;
    
    free(queue);
    return 0;
}

int queue_enqueue(queue_t queue, void *data)
{
    if (queue == NULL || data == NULL)
        return -1;
    
    struct queue_node *new_node = malloc(sizeof(struct queue_node));
    if (new_node == NULL)
        return -1;
    
    new_node->data = data;
    new_node->next = NULL;
    
    if (queue->tail == NULL) { /* Empty queue */
        queue->head = new_node;
        queue->tail = new_node;
    } else {
        queue->tail->next = new_node;
        queue->tail = new_node;
    }
    
    queue->length++;
    return 0;
}

int queue_dequeue(queue_t queue, void **data)
{
    if (queue == NULL || data == NULL || queue->length == 0)
        return -1;
    
    struct queue_node *temp = queue->head;
    *data = temp->data;
    
    queue->head = temp->next;
    if (queue->head == NULL) /* Queue is now empty */
        queue->tail = NULL;
    
    free(temp);
    queue->length--;
    return 0;
}

int queue_delete(queue_t queue, void *data)
{
    if (queue == NULL || data == NULL || queue->length == 0)
        return -1;
    
    struct queue_node *current = queue->head;
    struct queue_node *prev = NULL;
    
    while (current != NULL) {
        if (current->data == data) {
            if (prev == NULL) { /* Deleting head */
                queue->head = current->next;
                if (queue->head == NULL) /* Queue is now empty */
                    queue->tail = NULL;
            } else {
                prev->next = current->next;
                if (current->next == NULL) /* Deleting tail */
                    queue->tail = prev;
            }
            
            free(current);
            queue->length--;
            return 0;
        }
        
        prev = current;
        current = current->next;
    }
    
    return -1; /* Data not found */
}

int queue_iterate(queue_t queue, queue_func_t func)
{
    if (queue == NULL || func == NULL)
        return -1;
    
    struct queue_node *current = queue->head;
    struct queue_node *next;
    
    while (current != NULL) {
        /* Save next pointer in case current node is deleted by func */
        next = current->next;
        func(queue, current->data);
        current = next;
    }
    
    return 0;
}

int queue_length(queue_t queue)
{
    if (queue == NULL)
        return -1;
    
    return queue->length;
}