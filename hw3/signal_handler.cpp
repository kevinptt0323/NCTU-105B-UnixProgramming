#include <signal.h>
#include <cstdio>

void sigint_handler(int sig) {
}

void sigtstp_handler(int sig) {
}

void init_signal_handler() {
	signal(SIGINT, SIG_IGN);
	signal(SIGTSTP, SIG_IGN);
	signal(SIGTTIN, SIG_IGN);
	signal(SIGTTOU, SIG_IGN);
}
