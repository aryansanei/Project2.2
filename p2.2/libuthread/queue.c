#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "queue.h"
typedef struct queue_node* queue_node_t;

struct queue_node {
    void* data;
    struct queue_node* next;
};

struct queue {
    struct queue_node* front;
    struct queue_node* back;
    int length;
};

queue_t queue_create(void)
{
	queue_t my_queue = (queue_t)malloc(sizeof(struct queue));
    if (my_queue == NULL) {
		return NULL;
    }

	my_queue->front = NULL;
	my_queue->back = NULL;
	my_queue->length = 0;

	return my_queue;
}

int queue_destroy(queue_t queue)
{
	if (queue == NULL || queue->length != 0) {
        return -1;
    }

    free(queue);
    queue = NULL;

    return 0;
}

int queue_enqueue(queue_t queue, void *data)
{
    if (queue == NULL || data == NULL) {
        return -1;
    }

    queue_node_t my_queue_node = (queue_node_t)malloc(sizeof(struct queue_node));
    if (my_queue_node == NULL) {
        return -1;
	}
	
	my_queue_node->data = data;
    my_queue_node->next = NULL;

    if (queue->length == 0) {
        queue->front = my_queue_node;
        queue->back = my_queue_node;    
    } else {
        queue->back->next = my_queue_node;
        queue->back = my_queue_node;
    }
    queue->length++;
	
	return 0;
}

int queue_dequeue(queue_t queue, void **data)
{
	if (queue == NULL || data == NULL || queue->length == 0) {
		return -1;
	}
	
	*data = queue->front->data;
	queue_node_t temp = queue->front;
	queue->front = queue->front->next;

	if (queue->front == NULL) {
		queue->back = NULL;
	}

	queue->length--;

	free(temp);

	return 0;
}

int queue_delete(queue_t queue, void *data)
{
	if (queue == NULL || data == NULL || queue->length == 0) {
		return -1;
	}

	queue_node_t current = queue->front;
	queue_node_t prev = NULL;

	while (current != NULL) {
		if (current->data == data) {
			if (prev == NULL) { // deleting front
				queue->front = current->next;
				if (queue->back == current)
					queue->back = NULL;
			} else {
				prev->next = current->next;
				if (queue->back == current)
					queue->back = prev;
			}
			free(current);
			queue->length--;
			return 0;
		}
		prev = current;
		current = current->next;
	}
	return -1; // not found
}

int queue_iterate(queue_t queue, queue_func_t func)
{
	if (queue == NULL || func == NULL) {
		return -1;
	}

	queue_node_t current = queue->front;
	while (current != NULL) {
		queue_node_t next = current->next;
		func(queue, current->data); // Run the callback
		current = next;
	}
	return 0;
}

int queue_length(queue_t queue)
{
	if (queue == NULL) {
        return -1;
    }

    return queue->length;
}
