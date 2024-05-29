#include "onSignal.h"
#include "queue.h"
#include "sharedmem.h"
#include <stdio.h>
#include <stdlib.h>

int workProducer = 1;

void stopProducer() {
    workProducer = 0;
}

void produce(struct SharedMemory *shared, struct Queue *queue) {
    onSignal(SIGUSR1, stopProducer);

    while (workProducer) {
        Message_p message = (Message_p) shmalloc(shared, sizeof(struct Message));
        message->type = 0;
        
        int size;

        do {
            size = rand() % 257;
        } while (size == 0);

        message->size = size;

        if (size == 256) {
            message->size = 0;
        }

        message->data = shmalloc(shared, size);
        
        for (int i = 0; i < size; i++)
            message->data[i] = rand();
        
        uint16_t hash = 0;

        for (int i = 0; i < size; i++) {
            for (int j = 0; j < 8; j+=3) {
                hash ^= ((uint16_t)message->data[i]) << j;
            }
        }

        message->hash = hash;
        int queueResult = 0;

        while ((queueResult = writeQueue(queue, message)) && workProducer)
            sleep(1);
        
        if (!workProducer && queueResult) {
            shfree(shared, message->data);
            shfree(shared, message);
            return;
        }

        uint64_t reads = getTotalQueueReads(queue);
        uint64_t writes = getTotalQueueWrites(queue);

        printf("Produced a message! Hash: %04x. Queue stats: R: %lu W: %lu\n", message->hash, reads, writes);

        if (!workProducer)
            return;
            
        sleep(1);
    }
}