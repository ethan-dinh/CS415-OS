#include "strheap.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define UNUSED __attribute__((unused))
#define TOTAL 50

int main(int argc, char *argv[]) {
    int i = 1;
    char *ptrs[TOTAL];
    bool ifdel = false;

    if (strcmp(argv[1], "-d") == 0) {
        ifdel = true;
	i = 2;
    }
    for ( ; i < argc; i++) {
	int k;
	for (k = 0; k < TOTAL; k++)
            ptrs[k] = str_malloc(argv[i]);
	if (ifdel)
	    for (k = 0; k < TOTAL; k++)
                str_free(ptrs[k]);
    }
    return EXIT_SUCCESS;
}

