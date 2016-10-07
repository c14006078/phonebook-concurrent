#define  _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <assert.h>

#include <unistd.h>
#include <pthread.h>
#include <sys/mman.h>

#include IMPL

#define DICT_FILE "./dictionary/words.txt"

static double diff_in_second(struct timespec t1, struct timespec t2)
{
    struct timespec diff;
    if (t2.tv_nsec-t1.tv_nsec < 0) {
        diff.tv_sec  = t2.tv_sec - t1.tv_sec - 1;
        diff.tv_nsec = t2.tv_nsec - t1.tv_nsec + 1000000000;
    } else {
        diff.tv_sec  = t2.tv_sec - t1.tv_sec;
        diff.tv_nsec = t2.tv_nsec - t1.tv_nsec;
    }
    return (diff.tv_sec + diff.tv_nsec / 1000000000.0);
}

int main(int argc, char *argv[])
{

    struct timespec start, end;
    double cpu_time1, cpu_time2;

#ifdef TIMING
    struct timespec mid;
#endif

#ifdef OPT
#include "file.c"
#include "debug.h"
#include <fcntl.h>
#define ALIGN_FILE "align.txt"

    /* align file */
    alignFile(DICT_FILE, ALIGN_FILE, MAX_LAST_NAME_SIZE);
#else
    FILE *fp;
    int i = 0;
    char line[MAX_LAST_NAME_SIZE];

    /* check file opening */
    fp = fopen(DICT_FILE, "r");
    if (!fp) {
        printf("cannot open the file\n");
        return -1;
    }
#endif

    /* build the entry */
    entry *pHead, *e;
    pHead = (entry *) malloc(sizeof(entry));

    printf("size of entry : %lu bytes\n", sizeof(entry));

    e = pHead;
    e->pNext = NULL;

#if defined(__GNUC__)
    __builtin___clear_cache((char *) pHead, (char *) pHead + sizeof(entry));
#endif

#if defined(OPT)

#ifndef THREAD_NUM

    const int thread_num = (int) sysconf(_SC_NPROCESSORS_ONLN);
#define THREAD_NUM thread_num

#endif

    clock_gettime(CLOCK_REALTIME, &start);

    /* map file context to memory space*/
    int fd = open(ALIGN_FILE, O_RDONLY | O_NONBLOCK);
    off_t fSize = getFileSize( ALIGN_FILE);

    char *map = mmap(NULL, fSize, PROT_READ, MAP_SHARED, fd, 0);
    assert(map && "mmap error");

    /* allocate at beginning */
    entry *entryPool = (entry *) malloc(sizeof(entry) * fSize / MAX_LAST_NAME_SIZE);
    assert( entryPool && "entry_pool error");

    /*pthread_setconcurrency(THREAD_NUM + 1);

    pthread_t *tid = (pthread_t *) malloc(sizeof( pthread_t) * THREAD_NUM);
    append_arg **arg = (append_arg **) malloc(sizeof(append_arg *) * THREAD_NUM);*/

    /* assign thread arg of append */
    /*for (int i = 0; i < THREAD_NUM; i++)
        arg[i] = new_append_arg(map + MAX_LAST_NAME_SIZE * i, map + fSize, i, THREAD_NUM, entryPool + i);
    #ifdef TIMING

    	clock_gettime(CLOCK_REALTIME, &mid);
    #endif*/

    /*TODO: thread pool*/
    /* muti-thread version append() */
    /**for (int i = 0; i < THREAD_NUM; i++)
        pthread_create( &tid[i], NULL, (void *) &append, (void *) arg[i]);*/

    //entry *last;

    /* sync wait for thread exit */
    /*for (int i = 0; i < THREAD_NUM; i++)
        pthread_join( tid[i], NULL);*/

    /* Merge */
    /*    for (int i = 0; i < THREAD_NUM; i++) {
            if (i == 0) {
                pHead = arg[i]->pHead;
                dprintf("Connect %d head string %s %p\n", i, arg[i]->pHead->lastName, arg[i]->mptr);
            } else {
                last->pNext = arg[i]->pHead;
                dprintf("Connect %d head string %s %p\n", i, arg[i]->pHead->lastName, arg[i]->mptr);
            }

            last = arg[i]->pLast;
            dprintf("Connect %d tail string %s %p\n", i, arg[i]->pLast->lastName, arg[i]->mptr);
            dprintf("round %d\n", i);
        }

        //show_entry( pHead);

        clock_gettime(CLOCK_REALTIME, &end);
        cpu_time1 = diff_in_second(start, end);

    #ifdef TIMING
        printf("execution time of append() only threads part: %lf sec\n", diff_in_second(mid, end));
    #endif*/

    /*Threadpool*/
#include "threadpool.h"

#define QUEUE_SIZE 100
#define APPEND_NUM 100

#ifdef TIMING
    clock_gettime(CLOCK_REALTIME, &mid);
#endif

    threadpool_t* pool = threadpool_create(THREAD_NUM, QUEUE_SIZE);

    /*Const for threadpool*/
    int index_num = fSize / MAX_LAST_NAME_SIZE;
    int task_num  = index_num / APPEND_NUM;
    int left_task = index_num % APPEND_NUM;

    int bound_offset = APPEND_NUM * MAX_LAST_NAME_SIZE;
    int last_offset = left_task * MAX_LAST_NAME_SIZE;

    /*allocate mem space*/
    append_arg **arg = (append_arg **) malloc(sizeof(append_arg *) * task_num);

    /*assign work arg*/
    int i;
    int err;
    for (i = 0; i < task_num; i++) {
        arg[i] = new_append_arg(map + bound_offset * i, map + bound_offset * (i+1), entryPool + APPEND_NUM * i);
        show_threadpool(pool); //Debug use
        while((err = threadpool_add( pool, append, (void *) arg[i])) != 0) {
            printf("i = %d, error num %d, arg->lastName = %s\n", i,err, arg[i]->mptr);
        }
    }

    /*check the left work*/
    if( left_task > 0) {
        arg[i] = new_append_arg(map + bound_offset * i, map + bound_offset * i + last_offset, entryPool + i * APPEND_NUM);
        while((err = threadpool_add( pool, (void *) &append, (void *) arg)) != 0)
            printf("error num %d\n", err);
    }

    assert( threadpool_destroy(pool) == 0);

    /* Merge */
    entry *last;
    for (int i = 0; i < task_num; i++) {
        if (i == 0) {
            pHead = arg[i]->pHead;
            dprintf("Connect %d head string %s %p\n", i, arg[i]->pHead->lastName, arg[i]->mptr);
        } else {
            last->pNext = arg[i]->pHead;
            dprintf("Connect %d head string %s %p\n", i, arg[i]->pHead->lastName, arg[i]->mptr);
        }

        last = arg[i]->pLast;
        dprintf("Connect %d tail string %s %p\n", i, arg[i]->pLast->lastName, arg[i]->mptr);
        dprintf("round %d\n", i);
    }

    //show_entry(pHead);
    show_threadpool(pool);

    clock_gettime(CLOCK_REALTIME, &end);
    cpu_time1 = diff_in_second(start, end);

#ifdef TIMING
    printf("execution time of append() only threads part: %lf sec\n", diff_in_second(mid, end));
#endif


#else
    clock_gettime(CLOCK_REALTIME, &start);
    while (fgets(line, sizeof(line), fp)) {
        while (line[i] != '\0')
            i++;
        line[i - 1] = '\0';
        i = 0;
        e = append(line, e);
    }

    clock_gettime(CLOCK_REALTIME, &end);
    cpu_time1 = diff_in_second(start, end);
#endif

#ifndef OPT
    /* close file as soon as possible */
    fclose(fp);
#endif

    e = pHead;

    /* the givn last name to find */
    char input[MAX_LAST_NAME_SIZE] = "zyxel";
    e = pHead;

    assert(findName(input, e) &&
           "Did you implement findName() in " IMPL "?");
    assert(0 == strcmp(findName(input, e)->lastName, "zyxel"));

#if defined(__GNUC__)
    __builtin___clear_cache((char *) pHead, (char *) pHead + sizeof(entry));
#endif
    /* compute the execution time */
    clock_gettime(CLOCK_REALTIME, &start);
    findName(input, e);
    clock_gettime(CLOCK_REALTIME, &end);
    cpu_time2 = diff_in_second(start, end);

    FILE *output;
#if defined(OPT)
    output = fopen("opt.txt", "a");
#else
    output = fopen("orig.txt", "a");
#endif
    fprintf(output, "append() findName() %lf %lf\n", cpu_time1, cpu_time2);
    fclose(output);

    printf("execution time of append() : %lf sec\n", cpu_time1);
    printf("execution time of findName() : %lf sec\n", cpu_time2);

#ifndef OPT
    if (pHead->pNext) free(pHead->pNext);
    free(pHead);
#else
    free(entryPool);
    //free(tid);
    free(arg);
    munmap(map, fSize);
#endif

    return 0;
}
