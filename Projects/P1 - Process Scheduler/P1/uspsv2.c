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
	int usr1_seen;
	bool alive;
} PCB;

// Initialize Global Variables
PCB proc_arr[MAX_PROGS];
int num_programs;
int active_processes;
int exec_prog = -1;
int quantum_time = -1;

// SIGUSR1 Tracker Variables
volatile int USR1_seen = 0;

/* Free Memory Helper Function */
void freePCB(int proc_ind) {
	for (int j = 0; proc_arr[proc_ind].args[j] != NULL; j++) {
		free(proc_arr[proc_ind].args[j]);
	} free(proc_arr[proc_ind].args);
}

/* Helper Function 
 * Inserts argument arrays into the global shared memory 
 */
int insertArgs(char *line, int prog_index) {
	// Parse the word and create an argument array
	PCB block;
	block.usr1_seen = false;
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

/* SIGNAL HANDLERS 
 * Establishing signal handlers for the parent and children
 */

/* SIGUSR1 Handler */
void sigusr1_handler(UNUSED int signum) {
	USR1_seen = 1;
}

/* Child Process Function */
void childProcess(int proc_ind) {
	// Indicate that the child is now alive
	PCB pcb = proc_arr[proc_ind];
	pcb.alive = true;	

	// Pause and wait for the signal
	struct timespec ms20 = {0, 20000000};
	while (!USR1_seen)
		(void) nanosleep(&ms20, NULL);

	// Execute the program
	char **args = pcb.args;
	execvp(args[0], args);

	// handle failed program
	char error[] = "Failed to execute program: ";
	p1strcat(error, args[0]);
	p1perror(2, error);
	for (int i = 0; i < num_programs; i++)
		freePCB(i);
	exit(-1); 
}

/* Initializes Child Processes */
void runProcesses(int count) {
	pid_t children[num_programs];
	int i;

	// Initialize Handlers
	if (signal(SIGUSR1, sigusr1_handler) == SIG_ERR)
		p1perror(2, "Failed to initialize SIGUSR1 signal!\n");

	// Launching each program
	for (long i = 0; i < count; i++, exec_prog++) {
		switch(children[i] = fork()) {
			case -1: 
				p1perror(2, "fork() failed\n");
				exit(EXIT_FAILURE);
				break; 
			case 0:
				childProcess(i); break;
		}
	}

	// Set the active processes
	active_processes = num_programs;

	// Sending SIGUSR1 signal
	for (i = 0; i < num_programs; i++)
		kill(children[i], SIGUSR1);
	
	// Sending interrupt signal
	for (i = 0; i < num_programs; i++)
		kill(children[i], SIGSTOP);

	// Sending continue signal
	for (i = 0; i < num_programs; i++)
		kill(children[i], SIGCONT);

	// Wait for all processes to finish
	for (int i = 0; i < count; i++)
		waitpid(-1, NULL, 0);
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
	exitStatus = EXIT_SUCCESS;

cleanup:
	for (int i = 0; proc_arr[i].args != NULL; i++) 	
		freePCB(i);
	return exitStatus;
}