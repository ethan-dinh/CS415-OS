/* Author: Ethan Dinh
 * Class: CS 415
 * Professor: Joe Sventek
 * Project 4 - Thread Safe String Heap
 * Date: Jun 4, 2023
 */

/* Importing Libraries */
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include "ADTs/hashcskmap.h"
#include "strheap.h"

#define CAPACITY 128

/* Global Lock & Map */
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
const CSKMap *map = NULL;

/* Define Structs */
typedef struct stringHeap {
    char *address;
    int refCount;
} StringHeap;

/* At Exit Function */
void cleanup() {
    if (map != NULL) map -> destroy(map);
	map = NULL;
} 

void freeValue(void *stringHeap) {
    StringHeap *sh = stringHeap;
    free(sh->address);
    free(sh);
}

/*  
 * "duplicate" `string' on the string heap  
 * returns the address of the heap allocated string or NULL if malloc() errors
 * increments the reference count for the string 
 */

char *str_malloc(char *string) {
    if (map == NULL) {
        atexit(cleanup);
        if ((map = HashCSKMap(CAPACITY, 5.0, freeValue)) == NULL) 
			return NULL;
    }

    // Lock the thread
    pthread_mutex_lock(&lock);

    // Check if the map contains the string
	StringHeap *sh = NULL;
    if (map -> get(map, string, (void **) &sh))
        sh->refCount++;
    else {
        // Make a new StringHeap
        sh = (StringHeap *) malloc(sizeof(StringHeap));
        sh -> address = strdup(string);
        sh -> refCount = 1;

        // Insert the StringHeap into the map
        map -> put(map, string, (void *)sh);
    } pthread_mutex_unlock(&lock);
	
	return sh->address;
}

/*  
 * "free" `string' on the string heap 
 * returns true if free'd, or false if the string was not present on the string heap 
 */
bool str_free(char *string) {
    bool status = false;

    // Lock the thread
    pthread_mutex_lock(&lock);

    // Check if the map contains the string
	StringHeap *sh;
    if ((map != NULL) && (map -> get(map, string, (void **) &sh))) {
        sh->refCount--;
        
        // Free if the refCount is 0
        if (sh->refCount == 0) map -> remove(map, string);
        status = true;
    } pthread_mutex_unlock(&lock);
    return status;
}

