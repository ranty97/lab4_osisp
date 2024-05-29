#pragma once
#include <unistd.h>
#include <pthread.h>

struct SharedMemory {
    void *memory;
    void *last_available;
    size_t size;
    pthread_mutex_t *mutex;
};

struct SharedMemoryBlock {
    int avaliable;
    size_t size;
};

struct SharedMemory initializeSharedMemory(size_t size);
void shfree(struct SharedMemory *mem, void *block);
void *shmalloc(struct SharedMemory *mem, size_t size);