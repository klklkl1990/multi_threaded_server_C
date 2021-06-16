#include "pcqueue.h"


/*
 * allocate a new producer-consumer work queue
 * @max_limit - limit on maximum size of the queue
 */
Queue *init_queue(int max_limit) {
    Queue *q = (Queue *) malloc(sizeof(Queue));
    if (!q) {
        return NULL;
    }
    q->head = q->tail = NULL;
    q->size = 0;
    q->curr_size = max_limit;
    return q;
}

/*
 * Get the current size of the queue
 * @q - pointer to the work-queue instance 
 */
int get_queue_size(Queue *q) {
    return q->size; /* Other threads might be concurrently updating size . Is it fine? */
}

/*
 * Get the capacity of the queue
 * @q - pointer to the work-queue instance
 */
int get_queue_capacity(Queue *q) {
    return q->curr_size;
}


/*
 * Add a generic job to the bottom of queue [production]( could be of any type (void*) )
 * That means this can even be a heterogeneous queue for practical purposes 
 * @q - pointer to a work-queue instance
 * @job - Job to be paused to the queue
 * @return - negative(failure), zero(success)
 */
int push(Queue *q, void *job) {
    QueueNode *next_job = (QueueNode *) malloc(sizeof(QueueNode));
    if (!next_job) {
        return -1;
    }
    int add_success = 0;

    next_job->next = NULL;
    next_job->data = job;
    if (q->size < q->curr_size) {
        /* If both head and tail are NULL before this insertion */
        if (NULL == q->tail)
            q->head = q->tail = next_job;
        else {
            /* Inserting into end of queue */
            q->tail->next = next_job;
            q->tail = q->tail->next;
        }
        q->size++;
        add_success = 1;
    }
    if (!add_success) {
        return -1;
    }
    return 0;
}

/*
 * Get a job from the top of the work queue [consumption]
 * @q - pointer to the work-queue instance
 */
void *pop(Queue *q) {
    QueueNode *next_job_node = NULL;
    void *job = NULL;
    next_job_node = q->head;
    if (q->head == q->tail) {
        q->head = q->tail = NULL;
    } else if (q->head != NULL) {
        q->head = q->head->next;
    }
    q->size--;
    if (next_job_node != NULL) {
        job = next_job_node->data;
        free(next_job_node);
    }
    return job;
}

/*
 * Free the existing queue
 * @q - pointer to the work-queue instance
 */
void delete_queue(Queue *q) {
    free(q);
}