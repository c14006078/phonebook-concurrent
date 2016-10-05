#ifndef _PHONEBOOK_H
#define _PHONEBOOK_H

#include <pthread.h>
#include <time.h>

#define MAX_LAST_NAME_SIZE 16

#define OPT 1

/**
 * 	STRUCTURE
 */
typedef struct _detail {
    char firstName[16];
    char email[16];
    char phone[10];
    char cell[10];
    char addr1[16];
    char addr2[16];
    char city[16];
    char state[2];
    char zip[5];
} detail;

typedef detail *dtlPtr;

typedef struct __PHONE_BOOK_ENTRY {
    char *lastName;
    struct __PHONE_BOOK_ENTRY *pNext;
    dtlPtr dtl;
} entry;


typedef struct _append_arg {
    char *mptr; 				///< mem ptr
    char *mbound; 			///< mem boundary
    int idx;						///< thread id
    int nthread;
    entry *pHead;
    entry *pLast;
} append_arg;

/**
 *		FUNCTION
 */
entry *findName(char lastname[], entry *pHead);

/**
 * Thread argument assign
 */
append_arg *new_append_arg(char *ptr, char *eptr, int tid, int ntd, entry *start);

/**
 * Pthread function with parallize
 */
void append(void *arg);

/**
 * Show entry context
 */
void show_entry(entry *pHead);


#ifdef DEBUG
/**
 * porting from main.c
 */
static double diff_in_second(struct timespec t1, struct timespec t2);
#endif

#endif
