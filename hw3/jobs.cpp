#include <signal.h>
#include "jobs.h"

void job::waitpid(int options=0) {
	for(pid_t job_pid: pid) {
		int status;
		::waitpid(job_pid, &status, options);
		if (WIFSTOPPED(status)) {
			background = true;
		}
	}
}
