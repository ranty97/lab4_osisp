#pragma once
#include <stdint.h>
#include "sharedmem.h"
#include <pthread.h>

struct Message {
    uint8_t type;
    uint16_t hash;
    uint8_t size;
    uint8_t *data;
};

typedef struct Message* Message_p;

struct Queue {
    pthread_mutex_t mutex;
    Message_p *head;
    Message_p *tail;
    Message_p *readCur;
    Message_p *writeCur;
    uint8_t full;
    uint64_t totalRead;
    uint64_t totalWrite;
};

struct Queue *createQueue(uint64_t capacity, struct SharedMemory *mem);
int readQueue(struct Queue* queue, Message_p *message);
int writeQueue(struct Queue* queue, Message_p message);
uint64_t getTotalQueueWrites(struct Queue* queue);
uint64_t getTotalQueueReads(struct Queue* queue);