#include "BXP/bxp.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#define UNUSED __attribute__((unused))
#define SERVICE "Callback"

void *svcFxn(void *args) {
    BXPService bxps = (BXPService)args;
    BXPEndpoint ep;
    char query[1024], *resp = "1";
    unsigned qlen;

    while ((qlen = bxp_query(bxps, &ep, query, 1024)) > 0) {
        // response always would be 1 if successful
        bxp_response(bxps, &ep, resp, 2);
        fputs(query, stdout);
    }
    return NULL;
}

int main(UNUSED int argc, UNUSED char *argv[]) {
    BXPConnection bxp;
    BXPService bxps;
    char response[10001];
    //char request[10000];
    Q_Decl(request, 10000);
    unsigned reqlen, rsplen;
    char ipaddr[16];
    unsigned short svcPort;
    pthread_t svcThread;
    char buf[BUFSIZ];

    assert(bxp_init(0, 1));
    bxp_details(ipaddr, &svcPort);// get details out
    assert((bxps = bxp_offer(SERVICE)));
    assert((bxp = bxp_connect("localhost", 5000, "Echo-Callback", 1, 1)));
    assert(! pthread_create(&svcThread, NULL, svcFxn, (void *)bxps));
    while (fgets(buf, BUFSIZ, stdin) != NULL) {
        sprintf(request, "%s|%u|%s|%s", ipaddr, svcPort, SERVICE, buf);
        reqlen = strlen(request) + 1;
        assert(bxp_call(bxp, Q_Arg(request), reqlen, response, 10000, &rsplen));
        if (response[0] == '0')
            fprintf(stderr, "Error response from server\n");
    }
    bxp_disconnect(bxp);
}
