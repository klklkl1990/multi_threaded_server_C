#ifndef __REQUEST_H__

#include "pcqueue.h"
typedef struct {
    int id, count, statics, dynamics;
} thread;

void requestHandle(int fd, long arrival, long dispatch, thread *worker);
long requestFileSize(int fd);

#endif
