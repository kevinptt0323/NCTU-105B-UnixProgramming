#include <signal.h>
#include "jobs.h"

int job::waitpid(int options=0) {
	bool all_exit = true;
	for(pid_t job_pid: pid) {
		int status;
		::waitpid(job_pid, &status, options);
		if (WIFSTOPPED(status)) {
			background = true;
			all_exit = false;
		}
	}
	return all_exit ? 0 : 1;
}
