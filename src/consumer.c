#include "onSignal.h"
#include "queue.h"
#include "sharedmem.h"
#include <stdio.h>

int workConsumer = 1;

void stopConsumer() {
    workConsumer = 0;
}

void consume(struct SharedMemory *shared, struct Queue *queue) {
    onSignal(SIGUSR1, stopConsumer);

    while (workConsumer) {
        Message_p message;
        
        while (readQueue(queue, &message) && workConsumer)
            sleep(1);

        if (!workConsumer)
            return;

        uint16_t hash = 0;

        int size = message->size;

        if (size == 0)
            size = 256;

        for (int i = 0; i < size; i++) {
            for (int j = 0; j < 8; j+=3) {
                hash ^= ((uint16_t)message->data[i]) << j;
            }
        }

        if (message->hash != hash)
            printf("WARNING!!! HASHES DIDNT MATCH!\n");

        uint64_t reads = getTotalQueueReads(queue);
        uint64_t writes = getTotalQueueWrites(queue);

        printf("Message consumed! Desired hash: %04x; Computed hash: %04x. Queue stats: R: %lu, W: %lu \n", message->hash, hash, reads, writes);
        shfree(shared, message->data);
        shfree(shared, message);
        sleep(1);
    }
}