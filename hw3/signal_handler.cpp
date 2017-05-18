#include <sys/wait.h>
#include <signal.h>
#include <cstdio>
#include "jobs.h"

extern jobs joblist;

void zombie_handler(int sig __attribute__((unused))) {
	pid_t pid;
	int status;
	while((pid = waitpid(-1, &status, WNOHANG)) > 0) {
		joblist.done_pid(pid);
	}
}

void init_signal_handler() {
	signal(SIGINT,  SIG_IGN);
	signal(SIGTSTP, SIG_IGN);
	signal(SIGTTIN, SIG_IGN);
	signal(SIGTTOU, SIG_IGN);
	signal(SIGCHLD, zombie_handler);
}
