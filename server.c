#include "segel.h"
#include "request.h"

// 
// server.c: A very, very simple web server
//
// To run:
//  ./server <portnum (above 2000)>
//
// Repeatedly handles HTTP requests sent to this port number.
// Most of the work is done within routines written in request.c
//
#define BLOCK 1
#define DROPTAIL 2
#define DROPHEAD 3
#define RANDOM 4

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t fill = PTHREAD_COND_INITIALIZER;
pthread_cond_t empty = PTHREAD_COND_INITIALIZER;

//the threadpool
pthread_t **thread_pool;

typedef struct {
    int fd;
    long size, arrival, dispatch;
} request;


int total_limit ;
int wait_limit;
int total_count;
int work_count;
int wait_count;

Queue * wait_queue;
int algorithm, threadid;

void
getargs(int *port, int *threads, int *buffers, int *alg, int argc, char *argv[])
{
    if (argc != 5) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(1);
    }
    if (atoi(argv[2]) <= 0) {
        fprintf(stderr, "Number of threads should be greater than 0!\n");
        exit(1);
    }
    if (atoi(argv[3]) <= 0) {
        fprintf(stderr, "Buffer size should be greater than 0!\n");
        exit(1);
    }
    if (strcmp(argv[4], "dh") != 0
        && strcmp(argv[4], "dt") != 0
        && strcmp(argv[4], "block") != 0
        && strcmp(argv[4], "random") != 0) {
        fprintf(stderr, "Buffer overload handle type not recognized!\n");
        exit(1);

    }
    *port = atoi(argv[1]);
    *threads = atoi(argv[2]);
    *buffers = atoi(argv[3]);
    if(strcasecmp(argv[4], "block") == 0) {
        *alg = BLOCK;
    }
    else if(strcasecmp(argv[4], "dt") == 0) {
        *alg = DROPTAIL;
    }
    else if(strcasecmp(argv[4], "dh") == 0) {
        *alg = DROPHEAD;
    }
    else {
        fprintf(stderr, "Usage: %s <port>]\n", argv[0]);
        exit(1);
    }
}

int requestcmp(const void *first, const void *second) {
    return ((*(request **)first)->size - (*(request **)second)->size);
}


void *consumer(void *arg) {
    thread worker;
    worker.id = -1;
    worker.count = 0;
    worker.statics = 0;
    worker.dynamics = 0;
    struct timeval dispatch;
    while(1) {
        pthread_mutex_lock(&lock);
        while(wait_count == 0) {
            pthread_cond_wait(&fill, &lock);
        }

        gettimeofday(&dispatch, NULL);
        if(worker.id < 0) {
            worker.id = threadid;
            //????????????? threadid++;
        }
        worker.count++;
        //liads' adding
        request *req= pop(wait_queue);
        req->dispatch = ((dispatch.tv_sec) * 1000 + dispatch.tv_usec/1000.0) + 0.5;
        wait_count--;
        work_count++;

        pthread_cond_signal(&empty);
        pthread_mutex_unlock(&lock);

        requestHandle(req->fd, req->arrival, req->dispatch, &worker);
        work_count--;
        total_count--;
        Close(req->fd);

    }
}

int main(int argc, char *argv[])
{
    int listenfd, connfd, port, threads, buffers, alg, clientlen;
    struct sockaddr_in clientaddr;
    struct timeval arrival;

    getargs(&port, &threads, &buffers, &alg, argc, argv);

    wait_limit = buffers;
    total_limit = buffers;
    algorithm = alg;

    wait_queue = init_queue(wait_limit);
    thread_pool = malloc(threads*sizeof(*thread_pool));

    int i;
    for(i = 0; i< threads; i++) {
        thread_pool[i] = malloc(sizeof(pthread_t));
        pthread_create(thread_pool[i], NULL, consumer, NULL);
    }

    //
    // Create some threads...
    //

    listenfd = Open_listenfd(port);
    while (1) {
        clientlen = sizeof(clientaddr);
        connfd = Accept(listenfd, (SA *)&clientaddr, (socklen_t *) &clientlen);
        gettimeofday(&arrival, NULL);

        pthread_mutex_lock(&lock);
        while(wait_count == wait_limit) {
            pthread_cond_wait(&empty, &lock);
        }
        request *req = malloc(sizeof(request));
        if(!req)
            return -1;
        req->size = requestFileSize(connfd);
        req->fd = connfd;
        req->arrival = ((arrival.tv_sec) * 1000 + arrival.tv_usec/1000.0) + 0.5;
        int flag = 0;
        while (wait_count+work_count == total_limit) {
            if (algorithm==BLOCK) {
                pthread_cond_wait(&fill, &lock);
            } else if (algorithm==DROPTAIL) {
                Close(connfd);
                pthread_mutex_unlock(&lock);
                flag = 1;
            } else if (algorithm==DROPHEAD) {
                request * curr_req = pop(wait_queue);
                Close(curr_req->fd);
                free(curr_req);
                wait_count--;
                total_count--;
            }
        }
        if(!flag) {
            push(wait_queue, req);
            wait_count++;
            total_count++;
            pthread_cond_signal(&fill);
            pthread_mutex_unlock(&lock);
        }
    }

}


    


 
