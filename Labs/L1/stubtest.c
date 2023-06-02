#include "ADTs/arraystack.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define UNUSED __attribute__((unused))

void log_and_die(char *s) {
	fprintf(stderr, "%s", s);
	exit(EXIT_FAILURE);
}

int main(UNUSED int argc, UNUSED char *argv[]) {
	const Stack *st;
	char buf[BUFSIZ];
	char *s;

	st = ArrayStack(0L, free);
	if (st == NULL)
		log_and_die("ArrayStack() failed\n");
	while (fgets(buf, sizeof buf, stdin) != NULL) {
		char *s = strdup(buf);
		if (! st->push(st, (void *)s))
			log_and_die("push() failed\n");
	}

	while (st->pop(st, (void **)&s)) {
		printf("%s", s);
		free(s);
	}
	st->destroy(st);
	return EXIT_SUCCESS;
}
