#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <stdbool.h>
#define UNUSED __attribute__((unused))

volatile int USR1_count = 0;

#define DEFAULT_COUNT 1

void onusr1(UNUSED int sig) {
	USR1_count++;
}

int main(int argc, char *argv[]) {
	struct timespec ms20 = {0, 20000000};
	int count = 5;
	if (signal(SIGUSR1, onusr1)==SIG_ERR) {
		fprintf(stderr, "Can't establish the SIGUSR1 handler\n");
		return EXIT_FAILURE;
	}
	while (USR1_count < count) {
		(void)nanosleep(&ms20, NULL);
	}
	printf("Signal received %d times\n", count);
	return EXIT_SUCCESS;
}
