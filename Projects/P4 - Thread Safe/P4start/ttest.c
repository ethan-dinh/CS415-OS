#include "table.h"
#include "row.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define UNUSED __attribute__((unused))

void freeRow(void *x) {
    const Row *r = (const Row *)x;
    r->destroy(r);
}

int main(UNUSED int argc, UNUSED char *argv[]) {
    const Table *t = Table_create(freeRow);
    char buf[BUFSIZ];
    unsigned long i, N;

    while (fgets(buf, BUFSIZ, stdin) != NULL) {
        const Row *r;
        char *p = strrchr(buf, '\n');
	if (p != NULL)
            *p = '\0';
	r = Row_create(buf);
	t->append(t, r);
    }
    N = t->size(t);
    for (i = 0UL; i < N; i++) {
        Row *r;
	t->retrieve(t, i, &r);
	r->csvline(r, buf);
        printf("%s\n", buf);
    }
    t->destroy(t);
    return EXIT_SUCCESS;
}
