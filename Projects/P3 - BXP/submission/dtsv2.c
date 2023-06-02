/* Name: Ethan Dinh
 * Class: CS 415 - Operating Systems
 * Professor: Joe Sventek
 * Project 3 - BXP (Version 2)
 */

/* Importing Libraries */
#include "BXP/bxp.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>

/* Definitions */
#define UNUSED __attribute__((unused))
#define SERVICE "DTS"
#define PORT 19999
#define HOST "localhost"
#define MAX_ARGS 100

/* Define Constants */
static const char *LEGAL[3] = {"OneShot", "Repeat", "Cancel"};
const int NUM_ARGS[3] = {7, 9, 2};

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

/* Checks if the query is legal */
int legalQuery(char *query) {
    char *words[MAX_ARGS];
    int n = extractWords(query, "|", words);
    
    // Check if the first word and the number of arguments is valid
    for (int i = 0; i < 3; i++)
        if ((strcmp(words[0], LEGAL[i]) == 0) && (n == NUM_ARGS[i]))
                return 1;
    return 0;
}

/* Thread Functions */
void *svcFxn(void *args) {
    BXPService bxps = (BXPService)args;
    BXPEndpoint ep;
    char query[10000], response[10001], testQuery[10000];
    unsigned qlen, rlen;
    int status;

    while ((qlen = bxp_query(bxps, &ep, query, 10000)) > 0) {
        // Appending the status byte to response
        strcpy(testQuery, query);
        status = legalQuery(testQuery);
        sprintf(response, "%d%s", status, query);

        // Echoing the query back to the client
        rlen = strlen(response) + 1;
        bxp_response(bxps, &ep, &response, rlen);

        // Report the query to stdout
        printf("Response: %s", response);
    } return NULL;
}

int main(UNUSED int argc, UNUSED char *argv[]) {
    // Initialize BXP runtime
    BXPService bxps;
    assert(bxp_init(PORT, 1));
    assert((bxps = bxp_offer(SERVICE)));
    printf("%s service initialized on %s:%d\n", SERVICE, HOST, PORT);

    // Initialize the service thread
    pthread_t svcThread;
    assert(!pthread_create(&svcThread, NULL, svcFxn, (void *)bxps));
    pthread_join(svcThread, NULL);
}
