#include <signal.h>
#include <cstdio>

void sigint_handler(int sig) {
}

void sigtstp_handler(int sig) {
}

void init_signal_handler() {
	signal(SIGINT, sigint_handler);
	signal(SIGTSTP, sigtstp_handler);
	//signal(SIGTTOU, SIG_IGN);
	//signal(SIGTTIN, SIG_IGN);
}
