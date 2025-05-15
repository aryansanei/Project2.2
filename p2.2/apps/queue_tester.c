#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include <queue.h>

#define TEST_ASSERT(assert)                 \
do {                                        \
    printf("ASSERT: " #assert " ... ");     \
    if (assert) {                           \
        printf("PASS\n");                   \
    } else  {                               \
        printf("FAIL\n");                   \
        exit(1);                            \
    }                                       \
} while(0)

/* Create */
void test_create(void)
{
    fprintf(stderr, "*** TEST create ***\n");

    TEST_ASSERT(queue_create() != NULL);
}

/* Enqueue/Dequeue simple */
void test_queue_simple(void)
{
    int data = 3, *ptr;
    queue_t q;

    fprintf(stderr, "*** TEST queue_simple ***\n");

    q = queue_create();
    queue_enqueue(q, &data);
    queue_dequeue(q, (void**)&ptr);
    TEST_ASSERT(ptr == &data);
    TEST_ASSERT(queue_length(q) == 0);
    queue_destroy(q);
}

/* Multiple enqueue/dequeue */
void test_queue_multiple(void)
{
    int data1 = 1, data2 = 2, data3 = 3;
    int *ptr;
    queue_t q;

    fprintf(stderr, "*** TEST queue_multiple ***\n");

    q = queue_create();
    
    queue_enqueue(q, &data1);
    queue_enqueue(q, &data2);
    queue_enqueue(q, &data3);
    
    TEST_ASSERT(queue_length(q) == 3);
    
    queue_dequeue(q, (void**)&ptr);
    TEST_ASSERT(ptr == &data1);
    TEST_ASSERT(queue_length(q) == 2);
    
    queue_dequeue(q, (void**)&ptr);
    TEST_ASSERT(ptr == &data2);
    TEST_ASSERT(queue_length(q) == 1);
    
    queue_dequeue(q, (void**)&ptr);
    TEST_ASSERT(ptr == &data3);
    TEST_ASSERT(queue_length(q) == 0);
    
    queue_destroy(q);
}

/* Test queue_delete */
void test_queue_delete(void)
{
    int data1 = 1, data2 = 2, data3 = 3;
    int *ptr;
    queue_t q;

    fprintf(stderr, "*** TEST queue_delete ***\n");

    q = queue_create();
    
    queue_enqueue(q, &data1);
    queue_enqueue(q, &data2);
    queue_enqueue(q, &data3);
    
    TEST_ASSERT(queue_length(q) == 3);
    
    /* Delete middle element */
    TEST_ASSERT(queue_delete(q, &data2) == 0);
    TEST_ASSERT(queue_length(q) == 2);
    
    /* Delete non-existent element */
    TEST_ASSERT(queue_delete(q, &data2) == -1);
    TEST_ASSERT(queue_length(q) == 2);
    
    /* Check remaining order */
    queue_dequeue(q, (void**)&ptr);
    TEST_ASSERT(ptr == &data1);
    
    queue_dequeue(q, (void**)&ptr);
    TEST_ASSERT(ptr == &data3);
    
    queue_destroy(q);
}

/* Counter for iterate test */
static int iterate_count = 0;

/* Callback function for iterate test */
static void iterate_func(queue_t q, void *data)
{
    int *value = (int*)data;
    
    /* If value is 2, delete it */
    if (*value == 2)
        queue_delete(q, data);
    
    iterate_count++;
}

/* Test queue_iterate */
void test_queue_iterate(void)
{
    int data1 = 1, data2 = 2, data3 = 3;
    int *ptr;
    queue_t q;

    fprintf(stderr, "*** TEST queue_iterate ***\n");

    q = queue_create();
    
    queue_enqueue(q, &data1);
    queue_enqueue(q, &data2);
    queue_enqueue(q, &data3);
    
    iterate_count = 0;
    queue_iterate(q, iterate_func);
    
    /* Should have iterated over all 3 elements */
    TEST_ASSERT(iterate_count == 3);
    
    /* Element 2 should have been deleted */
    TEST_ASSERT(queue_length(q) == 2);
    
    /* Check remaining order */
    queue_dequeue(q, (void**)&ptr);
    TEST_ASSERT(ptr == &data1);
    
    queue_dequeue(q, (void**)&ptr);
    TEST_ASSERT(ptr == &data3);
    
    queue_destroy(q);
}

/* Test queue_destroy */
void test_queue_destroy(void)
{
    int data = 1;
    queue_t q;

    fprintf(stderr, "*** TEST queue_destroy ***\n");

    q = queue_create();
    
    /* Destroying empty queue should succeed */
    TEST_ASSERT(queue_destroy(q) == 0);
    
    /* Create new queue with data */
    q = queue_create();
    queue_enqueue(q, &data);
    
    /* Destroying non-empty queue should fail */
    TEST_ASSERT(queue_destroy(q) == -1);
    
    /* Dequeue and try again */
    void *ptr;
    queue_dequeue(q, &ptr);
    TEST_ASSERT(queue_destroy(q) == 0);
}

int main(void)
{
    test_create();
    test_queue_simple();
    test_queue_multiple();
    test_queue_delete();
    test_queue_iterate();
    test_queue_destroy();

    printf("All tests passed!\n");
    return 0;
}