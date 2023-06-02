/* Name: Ethan Dinh
 * Class: CS 415 - Operating Systems
 * Professor: Joe Sventek
 * Project 3 - BXP (Version 1)
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

/* Thread Functions */
void *svcFxn(void *args) {
    BXPService bxps = (BXPService)args;
    BXPEndpoint ep;
    char query[10000], response[10001];
    unsigned qlen, rlen;

    while ((qlen = bxp_query(bxps, &ep, query, 10000)) > 0) {
        // Appending the status byte to response
        sprintf(response, "1%s", query);

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
    printf("%s hosted at %s:%d\n", SERVICE, HOST, PORT);

    // Initialize the service thread
    pthread_t svcThread;
    assert(!pthread_create(&svcThread, NULL, svcFxn, (void *)bxps));
    pthread_join(svcThread, NULL);
}
