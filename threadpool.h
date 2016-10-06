#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>

/*for thread*/
typedef struct {
    void (*func)(void *);
    void *arg;
} threadpool_task_t;

/*for threadpool*/
typedef struct {
    pthread_mutex_t lock;
    pthread_cond_t 	notify;
    pthread_t* 			threads;
    threadpool_task_t* queue;
    int thread_count;
    int queue_size;
    int head;
    int tail;
    int peding_task_count;
    int shutdown;//1 = true, 0 = false
    int started;
} threadpool_t;

/*for err flag*/
typedef enum {
    threadpool_normal_exit,
    threadpool_invalid,
    threadpool_lock_failure,
    threadpool_queue_full,
    threadpool_shutdown,
    threadpool_thread_failure,
} threadpool_task_err_t;

/*init thread pool*/
threadpool_t* threadpool_create(int thread_count, int queue_size);

//Thread in threadpool init and wait for job
static void *threadpool_thread(void *threadpool);

//Add task to threadpool
threadpool_task_err_t threadpool_add(threadpool_t *pool, void (*func)(void *), void *arg);

//free threadpool
//ret 0 = normal, 1 = failure
int threadpool_free(threadpool_t* pool);

//destory the threadpool by accident
threadpool_task_err_t threadpool_destroy(threadpool_t* pool);

