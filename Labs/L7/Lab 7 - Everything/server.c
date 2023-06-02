#include "BXP/bxp.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#define UNUSED __attribute__((unused))

int main(UNUSED int argc, UNUSED char *argv[]) {
    BXPEndpoint ep;
    BXPService svc;
    char query[10000], response[10001];
    unsigned qlen, rlen;

    assert(bxp_init(5000, 1));
    assert((svc = bxp_offer("Echo")));
    while ((qlen = bxp_query(svc, &ep, query, 10000)) > 0) {
        sprintf(response, "1%s", query);
        rlen = strlen(response) + 1;
	assert(bxp_response(svc, &ep, response, rlen));
    }
    return 0;
}
