#include "lab.h"
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdio.h>

/**
     * @brief opaque type definition for a queue
     */
    struct queue {
      void** data;
      int dequeueIndex;
      int enqueueIndex;
      int count;
      int capacity;
      bool shutdown;
      pthread_cond_t fill, empty;
      pthread_mutex_t dataLock;
    };
    /**
     * @brief Initialize a new queue
     *
     * @param capacity the maximum capacity of the queue
     * @return A fully initialized queue
     */
    queue_t queue_init(int capacity) {
      queue_t newQueue = malloc(sizeof(struct queue));
      if (!newQueue) { return NULL; }
      newQueue->capacity = capacity;
      newQueue->data = malloc(sizeof(void*) * newQueue->capacity);
      if (!newQueue->data) { return NULL; }
      newQueue->shutdown = false;
      newQueue->enqueueIndex = 0;
      newQueue->dequeueIndex = 0;
      newQueue->count = 0;
      newQueue->fill = (pthread_cond_t) PTHREAD_COND_INITIALIZER;
      newQueue->empty = (pthread_cond_t) PTHREAD_COND_INITIALIZER;
      newQueue->dataLock = (pthread_mutex_t) PTHREAD_MUTEX_INITIALIZER;
      return newQueue;
    }

    /**
     * @brief Frees all memory and related data signals all waiting threads.
     *
     * @param q a queue to free
     */
    void queue_destroy(queue_t q) {
      if (!q) { return; }
      pthread_mutex_destroy(&q->dataLock);
      pthread_cond_destroy(&q->fill);
      pthread_cond_destroy(&q->empty);
      free(q->data);
      free(q);
      return;
    }

    /**
     * @brief Adds an element to the back of the queue
     *
     * @param q the queue
     * @param data the data to add
     */
    void enqueue(queue_t q, void *data) {
      pthread_mutex_lock(&q->dataLock);
      while(q->count == q->capacity) {
        pthread_cond_wait(&q->empty, &q->dataLock);
      }
      q->data[q->enqueueIndex] = data;
      q->enqueueIndex = (q->enqueueIndex + 1) % q->capacity;
      q->count++;
      pthread_cond_signal(&q->fill);
      pthread_mutex_unlock(&q->dataLock);
      return;
    }

    /**
     * @brief Removes the first element in the queue.
     *
     * @param q the queue
     */
    void *dequeue(queue_t q) {
      pthread_mutex_lock(&q->dataLock);
      while(!is_shutdown(q) && q->count == 0) {
        pthread_cond_wait(&q->fill, &q->dataLock);
      }
      if (is_shutdown(q) && q->count == 0) {
        pthread_mutex_unlock(&q->dataLock);
        return NULL;
      }
      void* tmp = q->data[q->dequeueIndex];
      q->dequeueIndex = (q->dequeueIndex + 1) % q->capacity;
      q->count--;
      pthread_cond_signal(&q->empty);
      pthread_mutex_unlock(&q->dataLock);
      return tmp;
    }

    /**
     * @brief Set the shutdown flag in the queue so all threads can
     * complete and exit properly
     *
     * @param q The queue
     */
   void queue_shutdown(queue_t q) {
      pthread_mutex_lock(&q->dataLock);
      q->shutdown = 1;
      pthread_cond_broadcast(&q->fill);
      pthread_mutex_unlock(&q->dataLock);
      return;
   }

    /**
     * @brief Returns true is the queue is empty
     *
     * @param q the queue
     */
    bool is_empty(queue_t q) {
      return !q->count;
    }

    /**
     * @brief
     *
     * @param q The queue
     */
    bool is_shutdown(queue_t q) {
      return q->shutdown;
    }