#include "hashset.h"
#include "llistset.h"
#include "sort.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define UNUSED __attribute__((unused))

int scmp(void *x1, void *x2) {
    return strcmp((char *)x1, (char *)x2);
}

#define A 31L
long shash(void *x, long N) {
	int i;
	long sum = 0L;
	char *s = (char *)x;

	for (i = 0; s[i] != '\0'; i++)
		sum = A * sum + (long)s[i];
	return sum % N;
}

int main() {
	const Set *s = HashSet(free, scmp, 0L, 0.0, shash);
	char buf[BUFSIZ];
	
	long len, i, counter;
	counter = 1;
	while (fgets(buf, sizeof(buf), stdin) != NULL) {
		(void)s->add(s, (void*)strdup(buf)); 
		counter++;
	}
	
	char **array = (char **)s->toArray(s, &len);
	for (i = 0; i < len; i++)
		printf("%s ", array[i]);
	printf("... success\n");
	s->destroy(s);
}
