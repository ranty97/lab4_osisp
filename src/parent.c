#define _XOPEN_SOURCE 500

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <stdint.h>
#include "queue.h"
#include "sharedmem.h"
#include "producer.h"
#include "consumer.h"
#include <string.h>
#include <termios.h>
#include <sys/mman.h>

pid_t *producers = NULL;
size_t producersSize = 0;

pid_t *consumers = NULL;
size_t consumersSize = 0;

char getch()
{
    struct termios new_settings, stored_settings;
   
    tcgetattr(0,&stored_settings);
   
    new_settings = stored_settings;
     
    new_settings.c_lflag &= ~(ICANON | ECHO);
    new_settings.c_cc[VTIME] = 0;
    new_settings.c_cc[VMIN] = 1;
    
    tcsetattr(0, TCSANOW, &new_settings);
    char result = getchar();
    tcsetattr(0, TCSANOW, &stored_settings);
    return result;
}


void createProducer(struct SharedMemory *shared, struct Queue *queue) {
    pid_t pid = fork();

    if (pid == 0) {
        srand(time(NULL) * getpid());
        produce(shared, queue);
        exit(0);
    }

    producers = realloc(producers, (++producersSize) * sizeof(pid_t));
    producers[producersSize - 1] = pid;

    printf("Created new producer. Total producers: %ld\n", producersSize);
}

void killProducer() {
    if (producersSize == 0)
        return;
    
    producersSize--;
    kill(producers[producersSize], SIGUSR1);
    wait(NULL);

    if (producersSize)
        producers = realloc(producers, producersSize * sizeof(pid_t));
    else {
        free(producers);
        producers = NULL;
    }

    printf("Killed a producer. Total producers: %ld\n", producersSize);
}

void createConsumer(struct SharedMemory *shared, struct Queue *queue) {
    pid_t pid = fork();

    if (pid == 0) {
        consume(shared, queue);
        exit(0);
    }

    consumers = realloc(consumers, (++consumersSize) * sizeof(pid_t));
    consumers[consumersSize - 1] = pid;

    printf("Created new consumer. Total consumers: %ld\n", consumersSize);
}

void killConsumer() {
    if (consumersSize == 0)
        return;
    
    consumersSize--;
    kill(consumers[consumersSize], SIGUSR1);
    wait(NULL);
    
    if (consumersSize)
        consumers = realloc(consumers, consumersSize * sizeof(pid_t));
    else {
        free(consumers);
        consumers = NULL;
    }

    printf("Killed a consumer. Total consumers: %ld\n", consumersSize);
}

int main() {
    struct SharedMemory *shared = mmap(NULL, sysconf(_SC_PAGESIZE), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    (*shared) = initializeSharedMemory(sysconf(_SC_PAGESIZE) * 1024);

    struct Queue *queue = (void*)(shared + 1);

    (*queue) = *createQueue(100, shared);

    char action;

    while ((action = getch()) != 'q') {
        switch (action) {
            case 'o': createConsumer(shared, queue); break;
            case 'i': killConsumer(); break;
            case 'l': createProducer(shared, queue); break;
            case 'k': killProducer(); break;
        }
    }

    while (consumersSize)
        killConsumer();
    
    while (producersSize)
        killProducer();

    return 0;
}