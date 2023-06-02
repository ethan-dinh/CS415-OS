#include "BXP/bxp.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#define UNUSED __attribute__((unused))

int main(UNUSED int argc, UNUSED char *argv[]) {
    BXPConnection bxp;
    char response[10001];
    char request[10000];
    unsigned reqlen, rsplen;
    char buf[BUFSIZ];

    assert(bxp_init(0, 1));
    assert((bxp = bxp_connect("localhost", 5000, "Echo", 1, 1)));
    while (fgets(buf, BUFSIZ, stdin) != NULL) {
        strcpy(request, buf);
        reqlen = strlen(request) + 1;
        assert(bxp_call(bxp, request, reqlen, response, 10001, &rsplen));
        if (response[0] == '0')
            fprintf(stderr, "Error response from server\n");
        else
            fputs(response+1, stdout);
    }
    bxp_disconnect(bxp);
}
