#include <sys/wait.h>
#include "jobs.h"

int job::waitpid(int options=0) {
	bool all_exit = true;
	for(pid_t job_pid: pid) {
		int status;
		if (!job_pid) continue;
		::waitpid(job_pid, &status, options);
		if (!WIFEXITED(status)) {
			all_exit = false;
		}
		if (WIFSTOPPED(status)) {
			running = false;
		}
	}
	return all_exit ? 0 : 1;
}
