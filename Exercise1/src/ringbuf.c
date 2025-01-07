#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <ringbuf.h>

struct RingBuffer *rb_new(int size)
{
    struct RingBuffer *rb = malloc(sizeof(struct RingBuffer));

    rb->buffer = malloc(sizeof(int) * size);
    rb->capacity = size;
    rb->insertIdx = 0;
    rb->removeIdx = 0;
    rb->length = 0;

    int err = sem_init(&rb->full, 0, 0);
    if (err != 0)
    {
        free(rb);
        free(rb->buffer);
        return NULL;
    }
    err = sem_init(&rb->empty, 0, rb->capacity);
    if (err != 0)
    {
        (void)sem_destroy(&rb->full);
        free(rb);
        free(rb->buffer);
        return NULL;
    }
    err = pthread_mutex_init(&rb->mutex, NULL);
    if (err != 0)
    {
        (void)sem_destroy(&rb->full);
        (void)sem_destroy(&rb->empty);
        free(rb);
        free(rb->buffer);
        return NULL;
    }

    return rb;
}

void rb_print(const struct RingBuffer *const rb)
{
    // printf("b:%p c:%d, l:%d, i:%d, r:%d  ", rb->buffer, rb->capacity, rb->length, rb->insertIdx, rb->removeIdx);

    printf("[");
    int idx, elem;
    for (elem = 0, idx = rb->removeIdx; elem < rb->length; elem++, idx = (idx + 1) % rb->capacity)
    {
        printf("%d, ", rb->buffer[idx]);
    }

    printf("]\n");
}

void rb_push(struct RingBuffer *rb, int val)
{
    sem_wait(&rb->empty);
    pthread_mutex_lock(&rb->mutex);
    assert(rb->length < rb->capacity && "Bounds error: Attempted to push an element into a full buffer");
    rb->buffer[rb->insertIdx] = val;
    rb->insertIdx = (rb->insertIdx + 1) % rb->capacity;
    rb->length++;
    pthread_mutex_unlock(&rb->mutex);
    sem_post(&rb->full);
}

int rb_pop(struct RingBuffer *rb)
{
    sem_wait(&rb->full);
    pthread_mutex_lock(&rb->mutex);
    assert(rb->length > 0 && "Bounds error: Attempted to pop an element from an empty buffer");
    int val = rb->buffer[rb->removeIdx];
    rb->removeIdx = (rb->removeIdx + 1) % rb->capacity;
    rb->length--;
    pthread_mutex_unlock(&rb->mutex);
    sem_post(&rb->empty);

    return val;
}

void rb_destroy(struct RingBuffer *rb)
{
    pthread_mutex_destroy(&rb->mutex);
    sem_destroy(&rb->full);
    sem_destroy(&rb->empty);
    free(rb->buffer);
    free(rb);
}