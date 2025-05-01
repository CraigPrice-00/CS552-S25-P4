#include "lab.h"
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdio.h>

    // struct queue: the queue which implements the bounded buffer problem
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
    
    //CODE REVIEW: added more error handling
    /* queue_init: this function initializes a queue with a specified capacity
     * capacity: the capacity of the queue
     * queue_t: the initialized queue
     */
    queue_t queue_init(int capacity) {
      if (capacity == 0) {
        fprintf(stderr,"Error, capacity cannot be 0.");
      }
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

    //CODE REVIEW: added more error handling
    /* queue_destroy: this function destroys the queue and associated data
     * q: the queue to destroy
     */
    void queue_destroy(queue_t q) {
      if (!q) { 
        fprintf(stderr,"Error, null pointer passed to queue_destroy.\n"); 
      }
      pthread_mutex_destroy(&q->dataLock);
      pthread_cond_destroy(&q->fill);
      pthread_cond_destroy(&q->empty);
      free(q->data);
      free(q);
      return;
    }

    //CODE REVIEW: added more error handling
    /* enqueue: this function enqueues data at the back of the queue
     * q: the queue to add data to
     * data: the data to add
     */
    void enqueue(queue_t q, void *data) {
      if (!q || !data) { 
        fprintf(stderr,"Error, null pointer passed to enqueue.\n"); 
      }
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

    //CODE REVIEW: added more error handling
    /* dequeue - this function removes and returns the item at the front of the queue
     * q: the queue to dequeue from
     * returns the data at the front of the queue
     */
    void *dequeue(queue_t q) {
      if (!q) { return NULL; }
      pthread_mutex_lock(&q->dataLock);
      while(!is_shutdown(q) && is_empty(q)) {
        pthread_cond_wait(&q->fill, &q->dataLock);
      }
      if (is_shutdown(q) && is_empty(q)) {
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

    //CODE REVIEW: added more error handling
    /* queue_shutdown: this function sets the shutdown flag and signals all consumers to finish up
     * q: the queue to shutdown
     */
   void queue_shutdown(queue_t q) {
      if (!q) { 
        fprintf(stderr,"Error, null pointer passed to queue_shutdown.\n"); 
      }
      pthread_mutex_lock(&q->dataLock);
      q->shutdown = 1;
      pthread_cond_broadcast(&q->fill);
      pthread_mutex_unlock(&q->dataLock);
      return;
   }

    //CODE REVIEW: added more error handling
    /* is_empty: this function checks if the queue is empty
     * q: the queue to check
     * returns true if empty
     */
    bool is_empty(queue_t q) {
      if (!q) { return NULL; }
      return !q->count;
    }

    //CODE REVIEW: added more error handling
    /* is_shutdown: this function checks if the queue is shutdown
     * q: the queue to check
     * returns true if shut down
     */
    bool is_shutdown(queue_t q) {
      if (!q) { return NULL; }
      return q->shutdown;
    }