#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>
#include <time.h>
#include <stdbool.h>
#include <getopt.h>
#include "p1fxns.h"

#define MAX_PROGS 128
#define MAX_ARGS 10
#define UNUSED __attribute__((unused))

# define USAGE "usage: uspsv? [-q <quantum in msec>] [workload_file]\n"

// Declare a process control block struct
typedef struct pcb {
	char **args;
	bool alive;
} PCB;

// Initialize Global Variables
PCB proc_arr[MAX_PROGS];
int num_programs;
int quantum_time = -1;

/* Free Memory Helper Function */
void freePCB(int proc_ind) {
    for (int j = 0; proc_arr[proc_ind].args[j] != NULL; j++) {
        free(proc_arr[proc_ind].args[j]);
    } free(proc_arr[proc_ind].args);
    proc_arr[proc_ind].alive = false; // set the PCB as inactive
}

/* Helper Function 
 * Inserts argument arrays into the global shared memory 
 */
int insertArgs(char *line, int prog_index) {
	// Parse the word and create an argument array
	PCB block;
	int arg_index = 0, num_args = 0;

	// Removes the newline character
	int i = p1strchr(line, '\n');
	line[i] = '\0';
	
	// Count the number of lines
	char arg[BUFSIZ]; i = 0;
	while ((i = p1getword(line, i, arg)) != -1) num_args++; 

	// Allocate Memory for the argv array
	if ((block.args = (char **) malloc((num_args + 1) * sizeof(char *))) == NULL) {
		p1perror(2, "Failed to allocate memory for arguments!\n");
		return 0;
	}

	// Insert into the processes array
	for (i = 0, arg_index = 0; (i = p1getword(line, i, arg)) != -1; arg_index++)
		block.args[arg_index] = p1strdup(arg); 

	block.args[arg_index] = NULL;
	proc_arr[prog_index] = block;

	return 1;
}

/* Initializes Child Processes */
void runProcesses(int count) {
    pid_t children[num_programs];
    int status;

    // Launching each program
    for (long i = 0; i < count; i++) {
        switch(children[i] = fork()) {
            case -1: 
                p1perror(2, "fork() failed\n");
                exit(EXIT_FAILURE);
                break; 
            case 0:
                execvp(proc_arr[i].args[0], proc_arr[i].args); 
                char error[] = "Failed to execute program: ";
                p1strcat(error, proc_arr[i].args[0]);
                p1perror(2, error);
				
				for (int i = 0; i < num_programs; i++)
					freePCB(i);

                _exit(EXIT_FAILURE);
                break;
        }
    }

    // Wait for all processes to finish and free memory for the PCBs
    for (int i = 0; i < count; i++) {
        pid_t pid = waitpid(children[i], &status, 0);
        if (pid != -1 && WIFEXITED(status) && WEXITSTATUS(status) == EXIT_SUCCESS) {
            freePCB(i);
        } else freePCB(i);
    }
}


int main(int argc, char *argv[]) {
	bool exitStatus = EXIT_FAILURE;
	char buf[BUFSIZ];
	extern int opterr;
	int fd, opt;

	// Check to see if a file is provided
	if (argc > 4) {
		p1putstr(2, USAGE);
		goto cleanup;
	}

	// Configuring Quantum Time
	char *tmp = getenv("USPS_QUANTUM_MSEC");
	if (tmp != NULL)
		quantum_time = p1atoi(tmp);

	opterr = 0;
	while((opt = getopt(argc, argv, "q:")) != -1) {
		switch(opt) {
			case 'q': 
				if (!(quantum_time = p1atoi(optarg))) {
					p1putstr(2, "Incorrect format for quantum time!\n");
					goto cleanup;
				} break;
			default:
				p1putstr(2, "No quantum time provided!\n");
				goto cleanup;
				break;
		}
	}

	// Opening the input file
	if ((argc - optind) == 0)
		fd = 0;
	else if ((fd = open(argv[optind], 0)) == -1) {
		char error[] = "Could not open file: ";
		p1strcat(error, argv[optind]);
		p1perror(2, error);
		goto cleanup;
	}

	// Creating the argument and program arrays
	for (num_programs = 0; p1getline(fd, buf, sizeof buf) > 0; num_programs++) {
		if ((insertArgs(buf, num_programs)) == 0) {
			proc_arr[num_programs].args = NULL;
			goto cleanup;
		}
	}

	// Run the processes using a helper function
	runProcesses(num_programs);
	if (fd != 0) close(fd);
	exitStatus = EXIT_SUCCESS;

cleanup:
	return exitStatus;
}