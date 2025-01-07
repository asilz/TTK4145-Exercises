#ifndef RINGBUF_H
#define RINGBUF_H

#include <pthread.h>
#include <semaphore.h>

struct RingBuffer
{
    int *buffer;
    int capacity;

    int insertIdx;
    int removeIdx;
    int length;

    sem_t empty;
    sem_t full;
    pthread_mutex_t mutex;
};

struct RingBuffer *rb_new(int size);    // not thread safe
void rb_destroy(struct RingBuffer *rb); // not thread safe

void rb_push(struct RingBuffer *rb, int val); // thread safe
int rb_pop(struct RingBuffer *rb);            // thread safe

void rb_print(const struct RingBuffer *const rb); // not thread safe

#endif