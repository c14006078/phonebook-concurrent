#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "phonebook_opt.h"
#include "debug.h"


/*phonebook* new_phonebook( entry* ( *append_func)( char *, entry *), entry *( *find_func)( char *, entry *)){

	phonebook *phbk = ( phonebook *) malloc( sizeof( phonebook));

	phbk->pHead = NULL;
	phbk->pLast = NULL;
	phpk->append = append_func;
	phpk->find = find_func;

	return phpk;
}*/

/*entry *phonebook_findName( char lastname[], phonebook* phbk)
{
	return findName( lastname, phbk->pHead);
}*/

entry *findName(char lastname[], entry *pHead)
{
    size_t len = strlen( lastname);
    while (pHead != NULL) {
        if (strncasecmp(lastname, pHead->lastName, len) == 0
                && (pHead->lastName[len] == '\n' ||
                    pHead->lastName[len] == '\0')) {
            pHead->lastName = (char *) malloc( sizeof(char) * MAX_LAST_NAME_SIZE);
            memset(pHead->lastName, '\0', MAX_LAST_NAME_SIZE);
            strcpy(pHead->lastName, lastname);
            pHead->dtl = (dtlPtr) malloc( sizeof( detail));
            return pHead;
        }
        dprintf("find string = %s\n", pHead->lastName);
        pHead = pHead->pNext;
    }
    return NULL;
}

append_arg *new_append_arg( char *ptr, char *bound, int id, int nthread, entry *start)
{
    append_arg *arg = (append_arg *) malloc(sizeof(append_arg));

    arg->mptr = ptr;
    arg->mbound = bound;
    arg->idx = id;
    arg->nthread = nthread;

    arg->pHead = (arg->pLast = start);

    return arg;
}

void append(void *_arg)
{
    struct timespec start, end;
    double cpu_time;

    clock_gettime( CLOCK_REALTIME, &start);

    append_arg *arg = (append_arg *) _arg;
    /*
        int count = 0;///> debug for correction
        entry *entryPtr = arg->pHead;

        for (char *memPtr = arg->mptr; memPtr < arg->mbound;
                memPtr += MAX_LAST_NAME_SIZE * arg->nthread,
                 count++) {

    				entryPtr->lastName = memPtr;

            dprintf("thread %d append string = %s\n", arg->idx, entryPtr->lastName);

    				entryPtr = (entryPtr->pNext = entryPtr + arg->nthread);
        }*/
    /**/
    int count = 0;
    char *memPtr = arg->mptr;

    entry *ePtr = arg->pHead;
    while(memPtr < arg->mbound) {
        if( memPtr != arg->mptr)
            ePtr = (ePtr->pNext = ePtr + arg->nthread);

        ePtr->lastName = memPtr;
        memPtr += MAX_LAST_NAME_SIZE * arg->nthread;

        ePtr->pNext = NULL;
        count++;
        dprintf("thread %d append string = %s\n", arg->idx, ePtr->lastName);
    }
    /**/

    /* assign the pLast*/
    arg->pLast = ePtr;
//		arg->pLast = entryPtr;
//    entryPtr->pNext = NULL;

    //show_entry( arg->pHead);

    clock_gettime(CLOCK_REALTIME, &end);
    cpu_time = diff_in_second(start, end);

    dprintf("thread take %lf sec, count %d\n", cpu_time, count);

    pthread_exit(NULL);
}

void show_entry(entry *pHead)
{
    while (pHead != NULL) {
        printf("lastName = %s\n", pHead->lastName);
        pHead = pHead->pNext;
    }
}

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
