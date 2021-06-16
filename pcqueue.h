#ifndef __PCQUEUE_H__
#define __PCQUEUE_H__

#include "stdlib.h"
/* Internal node for generic work_queue */
typedef struct queue_node {
    void *data;
    struct queue_node *next;
} QueueNode;

/* generic work_queue */
typedef struct queue {
    QueueNode *head, *tail;
    int size, curr_size;
} Queue;

/* work-queue API functions */
Queue *init_queue(int); /* allocate a new work-queue */
int push(Queue *, void *); /* Insert a new job */
void *pop(Queue *);

int get_queue_size(Queue *q);

int get_queue_capacity(Queue *q);

void delete_queue(Queue *q);

#endif
