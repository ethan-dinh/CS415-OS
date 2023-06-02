#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <sys/stat.h>
#include <time.h>
#include <stdbool.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/select.h>

#include "p1fxns.h"
#include "pidDisplay.h"

#define MAX_PROCS 128

// Header
char *HEADER[] = {"PID", "COMMAND", "STATE", "VMPEAK", "VMSIZE", "SYSCR", "SYSCW", "UTIME", "STIME", "CPU TIME"};

/* Global Variables */
char **out_arr;
pid_t active_pid;
int *active_pipe;
int spaces[10];

/* SIGUSR2 Handler */
void sigusr2_handler(UNUSED int signum) {
	read(active_pipe[0], &active_pid, sizeof(active_pid));
}

int get_terminal_width() {
    struct winsize ws;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws);
    return ws.ws_col;
}

bool is_pid_running(pid_t pid) {
    char path[64] = "/proc/";
    char pid_str[20];
    p1itoa(pid, pid_str);

    p1strcat(path, pid_str);
    struct stat statbuf;
    return stat(path, &statbuf) == 0;
}

char* getProcInfo(char *pid, char *display_str) {
    char path[8192] = "/proc/"; 
    p1strcat(path, pid);
    p1strcat(path, "/status");

    int fd = open(path, 0);
    char buf[8192];
    int line = 0, n;

    p1strcpy(display_str, pid);
    p1strpack(display_str, spaces[0], ' ', display_str);
    while((n = p1getline(fd, buf, BUFSIZ)) > 0) {
        if(line == 0) {	// Name
            p1strcpy(buf, buf + 6);
            buf[p1strlen(buf) - 1] = '\0';
            p1strpack(buf, spaces[1], ' ', buf);
            p1strcat(display_str, buf);
        }
        if(line == 2) {	// State
            p1strcpy(buf, buf + 7);
            buf[p1strlen(buf) - 1] = '\0';
            p1strpack(buf, spaces[2], ' ', buf);
            p1strcat(display_str, buf);
        }
        if(line == 16) {	// VmPeak
            p1strcpy(buf, buf + 12);
            buf[p1strlen(buf) - 1] = '\0';
            p1strpack(buf, spaces[3], ' ', buf);
            p1strcat(display_str, buf);
        }
        if(line == 17) {	// VmSize
            p1strcpy(buf, buf + 12);
            buf[p1strlen(buf) - 1] = '\0';
            p1strpack(buf, spaces[4], ' ', buf);
            p1strcat(display_str, buf);
        } line++;
	} 
    
    close(fd);
    fd = -1; 
    
    // Print stats in /io
    p1strcpy(path, "/proc/");
    p1strcat(path, pid);
    p1strcat(path, "/io");

    fd = open(path, O_RDONLY); line = 0;
    if (fd != -1) {
        while((n = p1getline(fd, buf, BUFSIZ)) > 0) {
            if(line == 2) {	
                p1strcpy(buf, buf + 7);
                buf[p1strlen(buf) - 1] = '\0';
                p1strpack(buf, spaces[5], ' ', buf);
                p1strcat(display_str, buf);
            }
            
            if(line == 3) {	
                p1strcpy(buf, buf + 7);
                buf[p1strlen(buf) - 1] = '\0';
                p1strpack(buf, spaces[6], ' ', buf);
                p1strcat(display_str, buf);
            } line++;
        } 

        close(fd);
        fd = -1; 
    } 
    
    // Print stats in /stat
    p1strcpy(path, "/proc/");
    p1strcat(path, pid);
    p1strcat(path, "/stat");

    fd = open(path, O_RDONLY);
    if(fd != -1) {
        p1getline(fd, buf, BUFSIZ);
        int wc = 0, index = 0;
        char word[50], milli[50];
        int total_ticks = 0;
        while((index = p1getword(buf, index, word)) != -1) {
            if(wc == 13) { // UTIME
                total_ticks += p1atoi(word);
                double seconds = (double) p1atoi(word) / (double) sysconf(_SC_CLK_TCK);
                int int_seconds = (int) seconds;
                int milliseconds = (seconds - int_seconds) * 100;

                p1itoa(int_seconds, word);
                p1strcat(word, ":");
                p1itoa(milliseconds, milli);
                p1strcat(word, milli);
                p1strcat(word, "s");
                p1strpack(word, spaces[7], ' ', word);
                p1strcat(display_str, word);
            } if(wc == 14) { // STIME
                total_ticks += p1atoi(word);	
                double seconds = (double) p1atoi(word) / (double) sysconf(_SC_CLK_TCK);
                int int_seconds = (int) seconds;
                int milliseconds = (seconds - int_seconds) * 100;

                p1itoa(int_seconds, word);
                p1strcat(word, ":");
                p1itoa(milliseconds, milli);
                p1strcat(word, milli);
                p1strcat(word, "s");
                p1strpack(word, spaces[8], ' ', word);
                p1strcat(display_str, word);
                break;
            } wc++;
        } 

        // TOTAL CPU TIME
        double seconds = (double) total_ticks / (double) sysconf(_SC_CLK_TCK);
        int int_seconds = (int) seconds;
        int milliseconds = (seconds - int_seconds) * 100;

        p1itoa(int_seconds, word);
        p1strcat(word, ":");
        p1itoa(milliseconds, milli);
        p1strcat(word, milli);
        p1strcat(word, "s");
        p1strpack(word, spaces[9], ' ', word);
        p1strcat(display_str, word);

        close(fd);
        fd = -1; 
    } 
    
    return display_str;
}

void monitor(int num_programs, PCB *proc_arr, char *out, int term_width) {
    char display_str[1000];
    char pid_str[10];

    for (int i = 0; i < num_programs; i++) {
        p1itoa((int) proc_arr[i].pid, pid_str);
        if (is_pid_running(proc_arr[i].pid)) {
            getProcInfo(pid_str, display_str);
            if (proc_arr[i].pid == active_pid) 
                p1strcat(out, "\033[42m");
            else
                p1strcat(out, "\033[48;2;53;52;56m");

            p1strcat(out, display_str);
            p1strcat(out, "\n");
            p1strcat(out, "\033[0m");

        } else {
            p1strcat(out, "\033[41m");
            p1strpack(pid_str, spaces[0], ' ', display_str);
            p1strcat(out, display_str);
            p1strpack("FINISHED", spaces[1], ' ', display_str);
            p1strcat(out, display_str);
            for (int i = 2; i < 10; i++) {
                p1strpack("N/A", spaces[i], ' ', display_str);
                p1strcat(out, display_str);
            } p1strcat(out, "\n\033[0m");
        }         
    }

    char tmp[200] = "";
    p1strpack("", term_width, '-', tmp);
    p1strcat(out, tmp);
    p1strcat(out, "\n");
}

void _read_output(int num_programs, PCB *proc_arr) {
    char buffer[BUFSIZ];
    for (int i = 0; i < num_programs; i++) {
        if (!is_pid_running(proc_arr[i].pid)) {
            ssize_t len = read(proc_arr[i].pipe_fd[0], buffer, sizeof(buffer));
            buffer[len] = '\0';
            if (len > 0) out_arr[i] = p1strdup(buffer);
        }
    }
}

void output(int num_programs, PCB *proc_arr, char *out) {
    _read_output(num_programs, proc_arr);
    for (int i = 0; i < num_programs; i++) {
        if (i > 4) { break; }
        
        char output[1000] = "\033[32m\nOutput from child process ";
        char pid[10];
        char child_num[1];
        p1itoa((int) proc_arr[i].pid, pid);
        p1itoa(i+1, child_num);
        p1strcat(output, child_num);
        p1strcat(output, " (PID: ");
        p1strcat(output, pid);
        p1strcat(output, "):\n\033[0m");
        if (out_arr[i] != NULL) {
            p1strcat(out, output);
            p1strcat(out, out_arr[i]);
        }
    }
}

void cleanup(int num_programs, PCB *proc_arr) {
    for (int i = 0; i < num_programs; i++) {
        if (out_arr[i] != NULL) free(out_arr[i]);
    } free(out_arr);

    for (int i = 0; i < num_programs; i++) {
        for (int j = 0; proc_arr[i].args[j] != NULL; j++) {
            free(proc_arr[i].args[j]);
        } free(proc_arr[i].args);
    }
}

void outputHeader(int term_width, char *out) {
    // Set bold white text w/ dark background
    p1strcat(out, "\033[1;37;48;5;236m");
    
    char tmp[200] = "";
    p1strcpy(tmp, "OUTPUT FROM WORKLOAD - FIRST 5 OUTPUTS");
    p1strpack(tmp, term_width, ' ', tmp);
    p1strcat(out, tmp);

    p1strcat(out, "\n");
    p1strcat(out, "\033[0m"); // Reset text attributes
}

void mainHeader(char *out) {
    // Set bold white text w/ dark background
    p1strcat(out, "\033[1;37;48;5;236m");

    for (int i = 0; i < 10; i++)  {
        char temp_buf[100] = "";
        p1strpack(HEADER[i], spaces[i], ' ', temp_buf);
        p1strcat(out, temp_buf);
    }
        
    p1strcat(out, "\n");
    p1strcat(out, "\033[0m"); // Reset text attributes
    out[p1strlen(out)] = '\0';
}

void monitorDisplay(PCB *proc_arr, int num_programs, int display_pipe[]) {
    
    // Initializing Variables
    active_pipe = display_pipe;
    int term_width = get_terminal_width();

    if (term_width > 207) {
        p1putstr(1, "Please reduce terminal size!\n");
        exit(EXIT_FAILURE);
    }

    // Calculate the Space Buffer
    int total = 0;
    for (int i = 0; i < 10; i++) {
        spaces[i] = term_width * 10 / 100;
        total += spaces[i];
    } spaces[3] += term_width - total;

    // Initialize the SIGUSR2 Handler
    if (signal(SIGUSR2, sigusr2_handler) == SIG_ERR)
		p1perror(2, "Could not catch SIGUSR2!\n");

    // Notify the scheduler that the signal handler is set up
    char ready = 'r';
    write(display_pipe[1], &ready, sizeof(ready));

    // Initialize the output array
    out_arr = (char **)malloc(num_programs * sizeof(char *));
    for (int i = 0; i < num_programs; i++) {
        out_arr[i] = NULL;

        // Set the O_NONBLOCK flag on the reading end of each child pipe
        int flags = fcntl(proc_arr[i].pipe_fd[0], F_GETFL, 0);
        fcntl(proc_arr[i].pipe_fd[0], F_SETFL, flags | O_NONBLOCK);
    }

    p1putstr(1, "\033[2J");
    bool any_process_running;
    do {
        // Move the cursor to the top-left corner and clear the screen
        p1putstr(1, "\033[H");

        // Print the Header
        char out[12000] = "";
        mainHeader(out);

        // Check to see if there are processes still running
        any_process_running = false;
        for (int i = 0; i < num_programs; i++) {
            if (is_pid_running(proc_arr[i].pid)) {
                any_process_running = true;
                break;
            }
        }

        monitor(num_programs, proc_arr, out, term_width);

        outputHeader(term_width, out);
        output(num_programs, proc_arr, out);

        char tmp[250];
        p1strpack("", term_width, '-', tmp);
        p1strcat(out, tmp);

        p1putstr(1, out);

        usleep(100000);
    } while (any_process_running);

    cleanup(num_programs, proc_arr);
}