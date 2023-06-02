/*
 * implementation for array-based generic stack
 */

#include "ADTs/arraystack.h"
#include <stdlib.h>
#include <stdio.h>

// this structure will contain the instance specific data
typedef struct st_data {
    long capacity; //capacity of the array
    long next; //next index
    void **theArray; // double pointer of array
    void (*freeValue)(void *e); // free value function
} StData;

/*
 * local function - traverses stack, applying user-supplied function
 * to each element
 */

static void purge(StData *std) {
    long i;

    for (i = 0L; i < std->next; i++)
        std->freeValue(std->theArray[i]);       /* free elem storage */
}

static void st_destroy(const Stack *st) {
	StData *std = (StData *) st->self;
	purge(std);
	free(std->theArray);
	free(std);
	free((void *)st);
}

static void st_clear(const Stack *st) {
	StData *std = (StData *) st->self;
	purge(std);
}

static bool st_push(const Stack *st, void *element) {
	StData *std = (StData *) st->self;
	bool status = ((std->next + 1) < (std->capacity));
	if (status)
		std->theArray[std->next++] = element;
    
	return status;
}

static bool st_pop(const Stack *st, void **element) {
    StData *std = (StData *) st->self;
	bool status = (std->next > 0L);
	if (status)
		*element = std->theArray[--std->next];
	return status;
}

static bool st_peek(const Stack *st, void **element) {
    StData *std = (StData *) st->self;
	bool status = (std->next > 0L);
	if (status)
		*element = std->theArray[std->next - 1];
	
	return status;
}

static long st_size(const Stack *st) {
	StData *std = (StData *) st->self;
    return (std->next);
}

static bool st_isEmpty(const Stack *st) {
    StData *std = (StData *) st->self;
	return (std->theArray[0] == NULL);
}

/*
 * local function - duplicates array of void * pointers on the heap
 *
 * returns pointers to duplicate array or NULL if malloc failure
 */
static void **arrayDupl(StData *std) {
    void **tmp = NULL;

    if (std->next > 0L) {
        size_t nbytes = std->next * sizeof(void *); //because next is the capacity
        tmp = (void **)malloc(nbytes);
        if (tmp != NULL) {
            long i, j = 0L; //to 0

            for (i = std->next - 1; i >= 0L; i--) //going backwards
                tmp[j++] = std->theArray[i];
        }
    }
    return tmp;
}

static void **st_toArray(const Stack *st, long *len) {
    StData *std = (StData *)st->self;
    void **tmp = arrayDupl(std);
    if (tmp != NULL)
        *len = std->next; //setting len address
    return tmp;
}

static const Iterator *st_itCreate(const Stack *st) {
    StData *std = (StData *)st->self;
    const Iterator *it = NULL;
    void **tmp = arrayDupl(std);

    if (tmp != NULL) {
        it = Iterator_create(std->next, tmp);
        if (it == NULL)
            free(tmp);
    }
    return it;
}

static const Stack *st_create(const Stack *st);

static Stack template = {
    NULL, st_create, st_destroy, st_clear, st_push, st_pop, st_peek, st_size,
    st_isEmpty, st_toArray, st_itCreate
};

/*
 * helper function to create a new Stack dispatch table
 */
static const Stack *newStack(long capacity, void (*freeValue)(void *e)){
    Stack *st = (Stack *)malloc(sizeof(Stack));

    if (st != NULL) {
        StData *std = (StData *)malloc(sizeof(StData));

        if (std != NULL) {
            long cap;
            void **array = NULL;

            cap = (capacity <= 0L) ? DEFAULT_STACK_CAPACITY : capacity;
            array = (void **)malloc(cap * sizeof(void *));
            if (array != NULL) {
                std->capacity = cap;
                std->next = 0L;
                std->theArray = array;
                std->freeValue = freeValue;
                *st = template;
                st->self = std;
            } else {
                free(std);
                free(st);
                st = NULL;
            }
        } else {
            free(st);
            st = NULL;
        }
    }
    return st;
}
 
static const Stack *st_create(const Stack *st) {
    StData *std = (StData *)st->self;

    return newStack(DEFAULT_STACK_CAPACITY, std->freeValue);
}

const Stack *ArrayStack(long capacity, void (*freeValue)(void *e)) {
    return newStack(capacity, freeValue);
}

const Stack *Stack_create(void (*freeValue)(void *e)) {
    return newStack(DEFAULT_STACK_CAPACITY, freeValue);
}
