#include "sharedmem.h"
#include <sys/mman.h>

struct SharedMemory initializeSharedMemory(size_t size) {
    struct SharedMemory mem = {
        .last_available = NULL,
        .size = size,
        .memory = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0),
        .mutex = NULL
    };

    pthread_mutex_t mutex;
    mem.mutex = &mutex;
    pthread_mutex_init(mem.mutex, NULL);
    pthread_mutex_t *new = shmalloc(&mem, sizeof(pthread_mutex_t));
    pthread_mutex_destroy(mem.mutex);
    mem.mutex = new;

    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);

    pthread_mutex_init(mem.mutex, &attr);

    return mem;
}

void shfree(struct SharedMemory *mem, void *block) {
    pthread_mutex_lock(mem->mutex);
    if (mem->memory + sizeof(struct SharedMemoryBlock) > block || mem->memory + mem->size - 1 < block) {
        pthread_mutex_unlock(mem->mutex);
        return;
    }
    
    ((struct SharedMemoryBlock*)(block - sizeof(struct SharedMemoryBlock)))->avaliable = 1;
    pthread_mutex_unlock(mem->mutex);
}

void *shmalloc(struct SharedMemory *mem, size_t size) {
    pthread_mutex_lock(mem->mutex);
    if (mem->last_available == NULL) {
        mem->last_available = mem->memory;
        struct SharedMemoryBlock* block = mem->memory;
        block->avaliable = 0;
        block->size = size;

        pthread_mutex_unlock(mem->mutex);
        return (void*)block + sizeof(struct SharedMemoryBlock);
    }

    struct SharedMemoryBlock* temp = mem->memory;

    while ((void*)temp < mem->memory + mem->size + sizeof(struct SharedMemoryBlock) && (void*)temp <= mem->last_available) {
        if(temp->avaliable && size <= temp->size) {
            temp->avaliable = 0;
            pthread_mutex_unlock(mem->mutex);
            return ((void*)temp) + sizeof(struct SharedMemoryBlock);
        }
        temp = ((void*)temp) + temp->size + sizeof(struct SharedMemoryBlock);
    }

    if ((void*)temp + sizeof(struct SharedMemoryBlock) + size < mem->memory + mem->size) {
        mem->last_available = temp;
        struct SharedMemoryBlock* block = mem->last_available;
        block->avaliable = 0;
        block->size = size;

        pthread_mutex_unlock(mem->mutex);
        return (void*)block + sizeof(struct SharedMemoryBlock);
    }

    pthread_mutex_unlock(mem->mutex);
    return NULL;
}