#include "p1fxns.h"
#include <stdlib.h>
#define UNUSED __attribute__((unused))

int main(UNUSED int argc, UNUSED char *argv[]) {
	char buf[4096], word[4096];
	while (p1getline(0, buf, sizeof buf) != 0) {
		int i = p1strchr(buf, '\n');
		buf[i] = '\0';
		i = 0;
		p1putstr(1, "|");
		while ((i = p1getword(buf, i, word)) != -1) {
			p1putstr(1, word);
			p1putstr(1, "|");
		}
		p1putstr(1, "\n");
	}		
	return EXIT_SUCCESS;
}
