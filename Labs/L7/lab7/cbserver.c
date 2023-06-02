#include "BXP/bxp.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#define UNUSED __attribute__((unused))

int extractWords(char *buf, char *sep, char *words[]) {
    int i;
    char *p;

    for (p = strtok(buf, sep), i = 0; p != NULL; p = strtok(NULL, sep), i++)
        words[i] = p;
    words[i] = NULL;
    return i;
}

int main(UNUSED int argc, UNUSED char *argv[]) {
    BXPEndpoint ep;
    BXPService svc;
    char query[10000], response[10001];
    unsigned qlen, rlen;

    assert(bxp_init(5000, 1));
    assert((svc = bxp_offer("Echo-Callback")));
    while ((qlen = bxp_query(svc, &ep, query, 10000)) > 0) {
        char *words[25];
	char out[BUFSIZ];
	char back[128];
	BXPConnection bxpc;
	unsigned port, backlen;
	(void) extractWords(query, "|", words);
        sscanf(words[1], "%u", &port);
	bxpc = bxp_connect(words[0], (unsigned short)port, words[2], 1, 1);
	strcpy(out, words[3]);
        bxp_call(bxpc, out, strlen(out)+1, back, sizeof back, &backlen);
	bxp_disconnect(bxpc);
        sprintf(response, "1%s", query);
        rlen = strlen(response) + 1;
	assert(bxp_response(svc, &ep, response, rlen));
    }
    return 0;
}
