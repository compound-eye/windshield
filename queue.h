#ifndef queue_h
#define queue_h

#include <pthread.h>


template<class Value, int capacity>
class Queue {
public:
    int size;
    bool quitting;

    Queue(): size(0), start(0), end(0), quitting(false) {
        pthread_mutex_init(&mutex, NULL);
        pthread_cond_init(&condNotFull, NULL);
        pthread_cond_init(&condNotEmpty, NULL);
    }

    ~Queue() {
        pthread_mutex_destroy(&mutex);
        pthread_cond_destroy(&condNotFull);
        pthread_cond_destroy(&condNotEmpty);
    }

    void Enqueue(const Value& value) {
        pthread_mutex_lock(&mutex);
        while (size >= capacity && ! quitting) {
            pthread_cond_wait(&condNotFull, &mutex);
        }
        if (size < capacity) {
            buffer[start] = value;
            ++size;
            ++start;
            if (start >= capacity) {
                start = 0;
            }
        }
        pthread_mutex_unlock(&mutex);
        pthread_cond_broadcast(&condNotEmpty);
    }

    Value Dequeue() {
        pthread_mutex_lock(&mutex);
        while (size <= 0 && ! quitting) {
            pthread_cond_wait(&condNotEmpty, &mutex);
        }
        Value value = buffer[end];
        if (size > 0) {
            buffer[end] = Value();
            --size;
            ++end;
            if (end >= capacity) {
                end = 0;
            }
        }
        pthread_mutex_unlock(&mutex);
        pthread_cond_broadcast(&condNotFull);
        return value;
    }

    void Quit() {
        if (! quitting) {
            quitting = true;
            pthread_cond_broadcast(&condNotEmpty);
            pthread_cond_broadcast(&condNotFull);
        }
    }

    int Capacity() const {return capacity;}

private:
    int start;
    int end;
    pthread_mutex_t mutex;
    pthread_cond_t condNotFull;
    pthread_cond_t condNotEmpty;
    Value buffer[capacity];
};

#endif /* queue_h */
