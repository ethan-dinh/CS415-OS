#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

void run_for_20_seconds(int time_length) {
    time_t start_time = time(NULL);
    time_t current_time = start_time;

    while (current_time - start_time < time_length) {
        // Do some work here...
        current_time = time(NULL);
    }
}

int main(int argc, char *argv[]) {
	if (argc != 2) {
		fprintf(stderr, "USAGE ERROR!\n");
		return 0;
	}
	
	int time_len = atoi(argv[1]);

    printf("Running for %d seconds...\n", time_len);
    run_for_20_seconds(time_len);
    printf("Done.\n");
    return 0;
}
