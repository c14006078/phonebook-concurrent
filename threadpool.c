#include "threadpool.h"

/*init thread pool*/
threadpool_t* threadpool_create(int thread_count, int queue_size)
{
    threadpool_t *pool = (threadpool_t *) malloc(sizeof(threadpool_t));

    //err input detect
    assert( "threadpool_create()"&& thread_count > 0 && queue_size > 0 );

    // initialize
    pool->thread_count = 0; //init thread count
    pool->queue_size = queue_size;
    pool->head = pool->tail = pool->peding_task_count = 0;
    pool->shutdown =  pool->started = 0;


    // init thread
    pool->threads = (pthread_t *) malloc(sizeof(pthread_t) * thread_count);
    pool->queue = (threadpool_task_t *) malloc(sizeof(threadpool_task_t) * queue_size);

    for(int i = 0; i < thread_count; i++) {
        if( pool->threads[i] =  pthread_create(&(pool->threads[i]), NULL, threadpool_thread, (void *) pool) != 0) {//return tid, not zero
            //threadpool_destroy(pool, 0);
            return NULL;
        }

        pool->thread_count++;
        pool->started++;
    }

    return pool;
}


//Thread in threadpool init and wait for job
static void *threadpool_thread(void *threadpool)
{
    threadpool_t *pool = (threadpool_t *) threadpool;
    threadpool_task_t task;

    while(1) {
        //mutex lock only one owner, it can be signal by cond var
        pthread_mutex_lock(&(pool->lock));

        //TODO: why e
        while((pool->peding_task_count == 0) && (!pool->shutdown)) {
            pthread_cond_wait(&(pool->notify), &(pool->lock));//wait for cond_sig
        }

        if(!(pool->shutdown) && (pool->peding_task_count == 0))
            break;

        /*after sig we can start task*/
        task.func = pool->queue[pool->head].func;//assign work
        task.arg = pool->queue[pool->head].arg;

        pool->head = (pool->head + 1) % pool->queue_size;//move the cur queue head to next
        pool->peding_task_count--;

        //release and let other thread to reach cond_wait
        pthread_mutex_unlock(&(pool->lock));

        //Go to work~
        (*(task.func))(task.arg);
    }

    pool->started--;

    pthread_mutex_unlock(&(pool->lock));//break point is above mutex_unlock
    pthread_exit(NULL);

    //return NULL;
}


//Add task to threadpool
threadpool_task_err_t threadpool_add(threadpool_t *pool, void (*func)(void *), void *arg)
{
    threadpool_task_err_t err;

    if( pool == NULL || func == NULL)
        return threadpool_invalid;

    if(pthread_mutex_lock(&(pool->lock)) != 0)
        return threadpool_lock_failure;

    do {
        // full?
        if(pool->peding_task_count == pool->queue_size) {
            err = threadpool_queue_full;
            break;
        }

        // shutdown?
        if(pool->shutdown) {
            err = threadpool_shutdown;
            break;
        }

        // Add task to queue
        pool->queue[pool->tail].func = func;
        pool->queue[pool->tail].arg = arg;
        pool->peding_task_count++;

        //cond_sig
        if(pthread_cond_signal(&(pool->notify))!= 0) {
            err = threadpool_lock_failure;
            break;
        }
    } while(0);

    // check unlock successs
    if(pthread_mutex_unlock(&pool->lock) != 0) {
        err = threadpool_lock_failure;
    }

    return err;
}

//free threadpool
//ret 0 = normal, 1 = failure
int threadpool_free(threadpool_t* pool)
{
    if(pool == NULL || pool->started > 0) {//check the any thread is runing
        return -1;
    }

    if( pool->threads) {
        free(pool->threads);
        free(pool->queue);

        pthread_mutex_lock(&(pool->lock));//Important to get the lock and destory it
        pthread_mutex_destroy(&(pool->lock));
        pthread_cond_destroy(&(pool->notify));
    }

    free(pool);
    return 0;
}

threadpool_task_err_t threadpool_destory(threadpool_t* pool)
{
    threadpool_task_err_t err = threadpool_normal_exit;

    if( pool == NULL)
        return threadpool_invalid;

    if(pthread_mutex_lock(&(pool->lock)) != 0)
        return threadpool_lock_failure;

    do {
        // already shutdown?
        if(pool->shutdown) {
            err = threadpool_shutdown;
            break;
        }

        if((pthread_cond_broadcast(&(pool->notify)) != 0) ||
                (pthread_mutex_unlock(&(pool->lock))) != 0) {
            err = threadpool_lock_failure;
            break;
        }

        //Join all worker threads
        for(int i = 0; i < pool->thread_count; i++)
            if(pthread_join(pool->threads[i], NULL) != 0)
                err = threadpool_thread_failure;
    } while(0);

    if(!err)
        threadpool_free(pool);

    return err;
}
