#include "queue.h"
#include <unistd.h>

struct Queue *createQueue(uint64_t capacity, struct SharedMemory *mem) {
    Message_p *buffer = shmalloc(mem, capacity * sizeof(Message_p));

    if(buffer == NULL)
        return NULL;
    
    struct Queue *queue = shmalloc(mem, sizeof(struct Queue));
    if (queue == NULL) {
        shfree(mem, buffer);
        return NULL;
    }

    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);

    pthread_mutex_init(&queue->mutex, &attr);
    queue->head = buffer;
    queue->tail = buffer + capacity;
    queue->readCur = buffer;
    queue->writeCur = buffer;
    queue->full = 0;

    return queue;
}

int readQueue(struct Queue* queue, Message_p *message) {
    pthread_mutex_lock(&queue->mutex);

	if(queue == NULL) {
        pthread_mutex_unlock(&queue->mutex);
        return 1;
    }
	
	if((queue->readCur == queue->writeCur) && !queue->full) {
        pthread_mutex_unlock(&queue->mutex);
        return 2;
    }
	
	*message = *(queue->readCur);

    Message_p *tmp = queue->readCur + 1;
	if(tmp >= queue->tail) 
        tmp = queue->head;
	if(tmp == queue->writeCur)
        queue->full = 0;
	queue->readCur = tmp;

    queue->totalRead++;
	pthread_mutex_unlock(&queue->mutex);

	return 0;
}

int writeQueue(struct Queue* queue, Message_p message) {
    pthread_mutex_lock(&queue->mutex);

    if(queue == NULL) {
        pthread_mutex_unlock(&queue->mutex);
        return 1;
    }
	
    if((queue->writeCur  == queue->readCur) && queue->full) {
        pthread_mutex_unlock(&queue->mutex);
        return 2;	
    }
	
	*(queue->writeCur) = message;
	
	Message_p *tmp = queue->writeCur + 1;
	if(tmp >= queue->tail)
        tmp = queue->head;
	if(tmp == queue->readCur)
        queue->full = 1;
	queue->writeCur = tmp;
	
    queue->totalWrite++;
    pthread_mutex_unlock(&queue->mutex);
	return 0;
}

uint64_t getTotalQueueWrites(struct Queue* queue) {
    pthread_mutex_lock(&queue->mutex);

    uint64_t totalWrites = queue->totalWrite;

    pthread_mutex_unlock(&queue->mutex);

	return totalWrites;
}

uint64_t getTotalQueueReads(struct Queue* queue) {
    pthread_mutex_lock(&queue->mutex);

    uint64_t totalReads = queue->totalRead;

    pthread_mutex_unlock(&queue->mutex);
    
	return totalReads;
}