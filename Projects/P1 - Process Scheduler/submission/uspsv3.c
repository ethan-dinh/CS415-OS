#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <time.h>
#include <stdbool.h>
#include <getopt.h>
#include "p1fxns.h"
#include "ADTs/queue.h"

#define MAX_PROGS 128
#define MAX_ARGS 10
#define UNUSED __attribute__((unused))
#define TICK_TIME 10
#define BUFSIZ 512

# define USAGE "usage: uspsv? [-q <quantum in msec>] [workload_file]\n"

// Declare a process control block struct
typedef struct pcb {
	pid_t pid;
	char **args;
	int usr1;
	int alive;
	int ticks;
} PCB;

// Initialize Global Variables
PCB proc_arr[MAX_PROGS];
PCB executing_proc;
int num_programs;
int active_processes;
int ticks = 0;
bool timer_running = true;

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
int initPCB(char *line, int prog_index) {
	// Parse the word and create an argument array
	PCB block;
	block.usr1 = 0;
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

	// Insert arguments into the PCB
	for (i = 0, arg_index = 0; (i = p1getword(line, i, arg)) != -1; arg_index++)
		block.args[arg_index] = p1strdup(arg); 

	block.args[arg_index] = NULL;
	
	// Initialize the number of ticks in Quantum Time
	block.ticks = ticks;
	block.alive = 1;
	proc_arr[prog_index] = block;

	return 1;
}

/* SIGUSR1 Handler */
void sigusr1_handler(UNUSED int signum) {
	USR1_seen = 1;
}

/* SIGCHLD Handler */
void chld_handler(UNUSED int signum) {
	pid_t pid;
	int status;
	while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
		if (WIFEXITED(status) || WIFSIGNALED(status)) {
			executing_proc.alive = 0;
			active_processes--;	
		}
	}	
}

/* Alarm Handler */
void alrm_handler(UNUSED int signum) {
	if ((--executing_proc.ticks == 0) || (!executing_proc.alive)) {
		timer_running = false;
		executing_proc.ticks = ticks;
		kill(executing_proc.pid, SIGSTOP);
	} return;
}

/* Helper Function to initialize signal handlers */
void initialize() {
	if (signal(SIGUSR1, sigusr1_handler) == SIG_ERR)
		p1perror(2, "Could not catch SIGUSR1!\n");

	if (signal(SIGCHLD, chld_handler) == SIG_ERR)
		p1perror(2, "Could not catch SIGCHLD!\n");

	if (signal(SIGALRM, alrm_handler) == SIG_ERR)
		p1perror(2, "Unable to catch SIGALRM\n");
}

/* Initialize timing interval */
void setTimer() {
	struct itimerval it_val;
	it_val.it_value.tv_sec = 0;
	it_val.it_value.tv_usec = 10000;
	it_val.it_interval = it_val.it_value;

	if (setitimer(ITIMER_REAL, &it_val, NULL) == -1)
		p1perror(2, "Error calling setitimer()\n");
}

/* Child Process Function */
void childProcess(int proc_ind) {
	PCB pcb = proc_arr[proc_ind];

	// Pause and wait for the signal
	struct timespec ms20 = {0, 20000000};
	while (!USR1_seen) 
		(void) nanosleep(&ms20, NULL);
		
	// Execute the program
	execvp(pcb.args[0], pcb.args);

	// handle failed program
	char error[] = "Failed to execute program: ";
	p1strcat(error, pcb.args[0]);
	p1perror(2, error);
	for (int i = 0; i < num_programs; i++)
		freePCB(i);
	exit(-1); 
}

/* Signal Delivery Helper Function */
void procScheduler(const Queue *schedule) {

	// Send each child SIGUSR1 - start each child
	long pid_index;
	setTimer();
	while (active_processes) {
		if (schedule -> dequeue(schedule, (void **) &pid_index)) {
			executing_proc = proc_arr[pid_index];
			if (!(proc_arr[pid_index].usr1++)) {
				kill(executing_proc.pid, SIGUSR1);
			} else 
				kill(executing_proc.pid, SIGCONT);
			
			// Start and wait for the timer to finish
			while(timer_running) 
				pause();
			timer_running = true;

			if (executing_proc.alive)
				schedule -> enqueue(schedule, (void *) pid_index);	
		}
	}
}

/* Initializes Child Processes */
void runProcesses(int count) {
	pid_t children[num_programs];
	const Queue *q = NULL;

	// Initialize Handlers
	initialize();

	// Set the active processes
	active_processes = num_programs;

	// Launching each program
	for (long i = 0; i < count; i++) {
		switch(children[i] = fork()) {
			case -1: 
				p1perror(2, "fork() failed\n");
				exit(EXIT_FAILURE);
				break; 
			case 0:
				childProcess(i); break;
			default:
				proc_arr[i].pid = children[i];
				break;
		}
	}

	// Initialize the scheduling queue
	q = Queue_create(doNothing);
	if (q == NULL) {
		p1putstr(2, "Could not create schedule queue!\n");
	}

	for (long i = 0; i < count; i++)
		q -> enqueue(q, (void *) i); 

	// Sending Signals
	procScheduler(q);

	if (q != NULL)
		q -> destroy(q);
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
	int quantum_time;
	char *tmp = getenv("USPS_QUANTUM_MSEC");
	if (tmp != NULL) quantum_time = p1atoi(tmp);

	opterr = 0;
	while((opt = getopt(argc, argv, "q:")) != -1) {
		switch(opt) {
			case 'q': 
				if (!(quantum_time = p1atoi(optarg))) {
					p1putstr(2, "Incorrect format for quantum time!\n");
					goto cleanup;
				} break;
			default:
				p1putstr(2, USAGE);
				goto cleanup;
		}
	}

	// Determining the number of ticks
	ticks = (int) quantum_time / TICK_TIME;

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
		if ((initPCB(buf, num_programs)) == 0) {
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