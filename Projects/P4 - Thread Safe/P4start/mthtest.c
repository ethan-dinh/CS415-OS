#include "strheap.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define UNUSED __attribute__((unused))
#define TOTAL 500

void *thr_func(void *args) {
    char *str = (char *)args;
    int k;
    char *ptrs[TOTAL];

    for (k = 0; k < TOTAL; k++)
        ptrs[k] = str_malloc(str);
    usleep(1000000);
    for (k = 0; k < TOTAL; k++)
        str_free(ptrs[k]);
    return NULL;
}

int main(int argc, char *argv[]) {
    pthread_t tids[50];
    int i, j = 0;
    void *status;

    for (i = 1; i < argc; i++) {
        pthread_create(tids+j, NULL, thr_func, (void *)argv[i]);
	j++;
    }
    for (i = 0; i < j; i++)
        pthread_join(tids[i], &status);
    return EXIT_SUCCESS;
}
