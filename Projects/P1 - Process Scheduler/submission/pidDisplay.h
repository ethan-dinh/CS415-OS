#ifndef _PIDDISPLAY_H_
#define _PIDDISPLAY_H_
#include <stdbool.h>

#define UNUSED __attribute__((unused))

typedef struct pcb {
	pid_t pid;
	char **args;
	int usr1;
	int alive;
	int ticks;
	int pipe_fd[2];
} PCB;

char* getProcInfo(char *pid, char *display_str);

void monitorDisplay(PCB *proc_arr, int num_programs, int display_pipe[]);

#endif	/* _PROC_DISPLAY_H_ */