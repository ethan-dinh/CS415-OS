/* Name: Ethan Dinh
 * Class: CS 415 - Operating Systems
 * Professor: Joe Sventek
 * Project 3 - BXP (Version 3)
 */

/* Importing Libraries */
#include "BXP/bxp.h"
#include "ADTs/prioqueue.h"
#include "ADTs/hashmap.h"
#include "ADTs/queue.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdint.h>
#include <sys/time.h>
#include <signal.h>
#include <valgrind/valgrind.h>

/* Definitions */
#define UNUSED __attribute__((unused))
#define SERVICE "DTS"
#define PORT 19999
#define HOST "localhost"
#define MAX_ARGS 100

/* Define Constants */
static const char *LEGAL[3] = {"OneShot", "Repeat", "Cancel"};
const int NUM_ARGS[3] = {7, 9, 2};

/* Define Threads, Global Locks, and Signals */
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
pthread_t receiveThread, procThread, timerThread;

/* Define a Global ADTs and Variables */
const Queue *procQ = NULL;
const Map *map = NULL;
const PrioQueue *pq = NULL;

/* Struct Declaration */
typedef struct event {
	unsigned long clid;
	char *host;
	char *service;
    unsigned port;
    unsigned long svid;
} Event;

/* Helper Functions 
 * Extracts words from a string and stores them in an array
 * Taken from Lab!
 */
int extractWords(char *buf, char *sep, char *words[]) {
    int i;
    char *p;
    for (p = strtok(buf, sep), i = 0; p != NULL; p = strtok(NULL, sep), i++)
        words[i] = p;
    words[i] = NULL;
    return i;
}

/* HashMap Functions */
long hash(void *key, long N) {
    uint64_t k = (uint64_t) key;
    return k % N;
}

void freeEvent(Event *e) {
    free(e->host);
    free(e->service);
    free(e);
}

/* Priority Queue CMP Function 
 * Compares two timeval tv objects and return 0
 * if p1 > p2 and 1 if p1 <= p2.
 */
int TimeCmp(void *p1, void *p2) {
    struct timeval *tv1, *tv2;
    int val1, val2;

    tv1 = (struct timeval *)p1;
    tv2 = (struct timeval *)p2;

    val1 = (int) (tv1->tv_sec * 1000) + (int) tv1->tv_usec / 1000;
    val2 = (int) (tv2->tv_sec * 1000) + (int) tv2->tv_usec / 1000;
    
    return val1 - val2;
}

/* Compares if two keys are equal */
int cmp(void *p1, void *p2) {
    return !((unsigned long) p1 == (unsigned long) p2);
}

/* Checks if the query is legal */
int legalQuery(char *query, char *args[]) {
    char *words[MAX_ARGS];
    int n = extractWords(query, "|", words);

    // Copy the contents of words to args
    for (int i = 0; i < n; i++)
        args[i] = words[i];

    // Check if the first word and the number of arguments is valid
    for (int i = 0; i < 3; i++)
        if ((strcmp(words[0], LEGAL[i]) == 0) && (n == NUM_ARGS[i]))
            return 1;
    return 0;
}

/* Thread Functions */
void *receiveFxn(void *args) {
    BXPService bxps = (BXPService)args;
    BXPEndpoint ep;
    char query[10000], response[10001], testQuery[10000];
    int status = 0;
    int qlen;
    unsigned long unique_id = 1;

    while ((qlen = bxp_query(bxps, &ep, query, 10000)) > 0) {
        char *args[MAX_ARGS]; 

        // Separate the query into words and check if it is legal
        strcpy(testQuery, query);
        status = legalQuery(testQuery, args);

        if(status) {
            // Report the received request
            printf("Received: %s\n", query);

            // Check if the query is OneShot
            if(strcmp(args[0], "OneShot") == 0) {
                // Malloc Memory for the Key and assign svid
                unsigned long svid = unique_id;

                // Create the response for OneShot
                sprintf(response, "%d%08lu", status, svid);

                // Initialize and malloc memory an Event
                Event *e = (Event *) malloc(sizeof(Event));
                if (e == NULL) {
                    fprintf(stderr, "ERROR: Unable to malloc memory for Event\n");
                    sprintf(response, "0%s", query);
                }

                // Populate the event struct pointer
                e->clid = (unsigned long) atol(args[1]);
                e->host = strdup(args[4]);
                e->service = strdup(args[5]);
                e->port = (unsigned) atoi(args[6]);
                e->svid = svid;

                // Convert time strings to timeval struct
                struct timeval *priority = malloc(sizeof(struct timeval));
                priority -> tv_sec = strtol(args[2], NULL, 10);
                priority -> tv_usec = strtol(args[3], NULL, 10);

                // Insert Event into priority queue and HashMap
                map -> putUnique(map, (void *) svid, (void *) e);
                pq -> insert(pq, (void *) priority, (void *) svid);
                unique_id++;
            } 
            
            // Check if the query is Cancel
            else if (strcmp(args[0], "Cancel") == 0) {
                // Create the response for OneShot
                sprintf(response, "%d%08lu", status, (unsigned long) atol(args[1]));

                // Cancel the event w/ SVID 
                Event *e; 
                unsigned long tmp_svid = (unsigned long) atol(args[1]);
                if (map -> get(map, (void *) tmp_svid, (void **) &e)) {
                    map -> remove(map, (void *) tmp_svid);
                    freeEvent(e);
                } else {
                    printf("ERROR: Cannot Cancel %ld - SVID does not exist!\n", tmp_svid);
                    sprintf(response, "0%s", query);
                }
            }

            // Check if the query is repeat
            else if (strcmp(args[0], "Repeat") == 0){
                printf("ERROR: Repeat is not implemented in V3!\n");
                sprintf(response, "0%s", query);
            }
            
        } else sprintf(response, "0%s", query); 

        VALGRIND_MONITOR_COMMAND("leak_check summary");

        bxp_response(bxps, &ep, &response, strlen(response) + 1);    
    } return NULL;
}

void responseFxn(Event *e) {
    BXPConnection bxpc;
    char request[10000], response[10001];
    unsigned reqlen, rsplen;

    // Create a BXP Connection
    bxpc = bxp_connect(e->host, e->port, e->service, 1, 1);
    reqlen = (unsigned) (sprintf(request, "%lu", e->clid) + 1);
    sprintf(response, "1%08lu", e->svid);
    bxp_call(bxpc, request, reqlen, response, sizeof(response), &rsplen);
    bxp_disconnect(bxpc);
}

void *procFxn() {
    while (1) {
        // Wait for the signal and check if the queue is empty
        pthread_mutex_lock(&lock);
        while (procQ -> isEmpty(procQ))
            pthread_cond_wait(&cond, &lock);
        pthread_mutex_unlock(&lock); 

        // Process the Events in the queue
        Event *e;
        while(procQ -> dequeue(procQ, (void **) &e)) {
            responseFxn(e); // THIS IS WHERE THE MEMORY LEAK IS FIX!
            freeEvent(e);
        } VALGRIND_MONITOR_COMMAND("leak_check summary");
    } return NULL;
}

void *timerFxn() {
    // Initialize a periodic timer that will signal processFxn to run
    struct timespec ms20 = {0, 20000000};

    // Pull the minimum out of the prio queue
    struct timeval *event_tv;
    struct timeval curr_tv;
    unsigned long svid;

    while (1) {
        (void) nanosleep(&ms20, NULL);
        pthread_mutex_lock(&lock);
        while (pq -> removeMin(pq, (void **) &event_tv, (void **) &svid)) {
            // Pull the time since epoch
            gettimeofday(&curr_tv, NULL);

            // Compare the two times
            if (TimeCmp((void *) event_tv, (void *) &curr_tv) < 0) {
                free(event_tv);
                Event *e; 
                if (map -> get(map, (void *) svid, (void **) &e)) {
                    map -> remove(map, (void *) svid);
                    fprintf(stdout, "Event Fired: %lu|%s|%s|%u\n", e->clid, e->host, e->service, e->port);
                    procQ -> enqueue(procQ, (void *) e);
                }
            } else {
                // Add the event back to the priority queue and map
                pq -> insert(pq, (void *) event_tv, (void *) svid);
                break;
            }
        }

        // Broadcast a signal and return the lock to the Process Thread
        pthread_cond_broadcast(&cond);
        pthread_mutex_unlock(&lock);

    } return NULL;
}

int main(UNUSED int argc, UNUSED char *argv[]) {
    BXPService bxps;
    int exitStatus = EXIT_FAILURE;

    // Initialize the Queues and Hashmap
    if ((pq = PrioQueue_create(TimeCmp, doNothing, doNothing)) == NULL) {
        fprintf(stderr, "Unable to create priority queue\n");
        goto cleanup;
    }

    if ((procQ = Queue_create(doNothing)) == NULL) {
        fprintf(stderr, "Unable to create process queue\n"); 
        goto cleanup;
    }

    if ((map = HashMap(0.0, 2.0, hash, cmp, doNothing, doNothing)) == NULL) {
        fprintf(stderr, "Unable to create Map to hold (key, value) pairs\n");
        goto cleanup;
    }

    // Initialize BXP runtime
    assert(bxp_init(PORT, 1));
    assert((bxps = bxp_offer(SERVICE)));
    printf("%s service initialized on (%s:%d):\n\n", SERVICE, HOST, PORT);

    // Initialize the service and process threads
    assert(!pthread_create(&receiveThread, NULL, receiveFxn, (void *)bxps));
    assert(!pthread_create(&procThread, NULL, procFxn, NULL));
    assert(!pthread_create(&timerThread, NULL, timerFxn, NULL));

    VALGRIND_MONITOR_COMMAND("leak_check summary");

    // Wait for the threads to finish
    pthread_join(receiveThread, NULL);
    pthread_join(procThread, NULL);
    pthread_join(timerThread, NULL);

    exitStatus = EXIT_SUCCESS;
    goto cleanup;

    cleanup:
        if (pq != NULL) pq->destroy(pq);
        if (procQ != NULL) procQ->destroy(procQ);
        if (map != NULL) map->destroy(map);
        return exitStatus;
}
