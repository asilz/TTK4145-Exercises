#include <stdio.h>
#include <ringbuf.h>

static int i = 0;

void *decrementingThread(void *arg)
{
    pthread_mutex_t *mutex = (pthread_mutex_t *)arg;
    for (size_t count = 0; count < 1000000; ++count)
    {
        pthread_mutex_lock(mutex);
        --i;
        pthread_mutex_unlock(mutex);
    }
    return NULL;
}

void *incrementingThread(void *arg)
{
    pthread_mutex_t *mutex = (pthread_mutex_t *)arg;
    for (size_t count = 0; count < 1000000; ++count)
    {
        pthread_mutex_lock(mutex);
        ++i;
        pthread_mutex_unlock(mutex);
    }
    return NULL;
}

void *producer(void *arg)
{
    struct RingBuffer *rb = (struct RingBuffer *)arg;
    for (int count = 0; count < 1000; ++count)
    {
        printf("[Producer]: pushing %d\n", count);
        rb_push(rb, count);
    }
    return NULL;
}

void *consumer(void *arg)
{
    struct RingBuffer *rb = (struct RingBuffer *)arg;
    for (int count = 0; count < 1000; ++count)
    {
        printf("[Consumer]: popping %d\n", rb_pop(rb));
    }
    return NULL;
}

int main(void)
{
    pthread_attr_t attribute;
    int err = pthread_attr_init(&attribute);
    if (err != 0)
    {
        return err;
    }

    pthread_mutex_t mutex;
    err = pthread_mutex_init(&mutex, NULL);
    if (err != 0)
    {
        return err;
    }

    pthread_t incrementing_pid;
    pthread_t decrementing_pid;

    err = pthread_create(&incrementing_pid, &attribute, incrementingThread, &mutex);
    if (err != 0)
    {
        return err;
    }
    err = pthread_create(&decrementing_pid, &attribute, decrementingThread, &mutex);
    if (err != 0)
    {
        return err;
    }

    err = pthread_join(incrementing_pid, NULL);
    if (err != 0)
    {
        return err;
    }
    err = pthread_join(decrementing_pid, NULL);
    if (err != 0)
    {
        return err;
    }

    err = pthread_mutex_destroy(&mutex);
    if (err != 0)
    {
        return err;
    }

    printf("Result = %d\n", i);

    struct RingBuffer *rb = rb_new(32);
    err = pthread_create(&incrementing_pid, &attribute, producer, rb);
    if (err != 0)
    {
        return err;
    }
    err = pthread_create(&decrementing_pid, &attribute, consumer, rb);
    if (err != 0)
    {
        return err;
    }

    err = pthread_attr_destroy(&attribute);
    if (err != 0)
    {
        return err;
    }

    err = pthread_join(incrementing_pid, NULL);
    if (err != 0)
    {
        return err;
    }
    err = pthread_join(decrementing_pid, NULL);
    if (err != 0)
    {
        return err;
    }

    rb_print(rb);
    rb_destroy(rb);

    return 0;
}
