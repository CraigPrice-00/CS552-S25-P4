#include "lab.h"
#include <pthread.h>
/**
     * @brief opaque type definition for a queue
     */
    struct queue {
      void** data;
      int startIndex;
      int count;
      int capacity;
      bool shutdown;
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
      newQueue->count = 0;
      newQueue->startIndex = 0;
      return newQueue;
    }

    /**
     * @brief Frees all memory and related data signals all waiting threads.
     *
     * @param q a queue to free
     */
    void queue_destroy(queue_t q) {
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
        q->data[(q->startIndex + q->count) % q->capacity] = data;
        q->count = (q->count + 1) % q->capacity;
        return;
    }

    /**
     * @brief Removes the first element in the queue.
     *
     * @param q the queue
     */
    void *dequeue(queue_t q) {
      void* returnVal = q->data[q->startIndex];
      q->startIndex = (q->startIndex + 1) % q->capacity;
      q->count--;
      return returnVal;
    }

    /**
     * @brief Set the shutdown flag in the queue so all threads can
     * complete and exit properly
     *
     * @param q The queue
     */
   void queue_shutdown(queue_t q) {
      q->shutdown = true;
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