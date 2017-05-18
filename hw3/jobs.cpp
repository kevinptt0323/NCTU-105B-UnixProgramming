#include <sys/wait.h>
#include <stdio.h>
#include "jobs.h"

void job::print(FILE* fout) const {
	const char* status = running ? "running" : (done ? "done" : "suspend");
	fprintf(fout, "%-15s\t%s\n", status, cmd.c_str());
}

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

void jobs::done_job(jobs::iterator itr) {
	size_t idx = itr-begin()+1;
	printf("done_job: %lu\n", idx);
	printf("%d\n", itr->foreground);
	if (!itr->foreground) {
		itr->done = true;
		fprintf(stdout, "[%lu]\t", idx);
		itr->print(stdout);
	}
	erase(itr);
}

void jobs::done_pid(pid_t pid) {
	printf("done_pid: %d\n", pid);
	for(auto itr=begin(); itr!=end(); ++itr) {
		if (itr->pid.back() == pid) {
			itr->running = false;
			done_job(itr);
			break;
		}
	}
}

void jobs::print(size_t idx, FILE* fout) const {
	if (idx>size()) return;
	const auto& curr_job = at(idx-1);
	fprintf(fout, "[%lu]", idx);
	for(auto pid: curr_job.pid) {
		fprintf(fout, " %d", pid);
	}
	fputs("\n", fout);
}
