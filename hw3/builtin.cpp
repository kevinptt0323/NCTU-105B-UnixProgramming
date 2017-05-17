#include <string>
#include <unordered_map>
#include <vector>
#include <unistd.h>
#include <signal.h>

#include "builtin.h"
#include "jobs.h"

#define add_builtin(name, ...) \
	builtins[name] = builtin(name, __VA_ARGS__)

#define UNUSED(var) var __attribute__((unused))

using std::string;
using std::unordered_map;
using std::vector;

unordered_map<string, builtin> builtins;

jobs joblist;

int shell_pid, shell_pgid;

builtin::builtin() {
	handler = NULL;
	funcid = 0;
}

builtin::builtin(const char *_name, HandlerFunc _handler, const char *_opts, int _funcid) {
	name = _name;
	handler = _handler;
	opts = _opts;
	funcid = _funcid;
}

builtin::~builtin() { }

int builtin::exec(const vector<string>& argv0) {
	char* argv[argv0.size()+1];
	for(size_t i=0; i<argv0.size(); i++) {
		argv[i] = (char*)argv0[i].c_str();
	}
	argv[argv0.size()] = NULL;

	return handler(name.c_str(), argv, opts.c_str(), funcid);
}

int builtin::exec(char *const argv[]) {
	return handler(name.c_str(), argv, opts.c_str(), funcid);
}


int bin_break(UNUSED(const char *name), UNUSED(char *const argv[]), UNUSED(const char *opts), UNUSED(const int& func)) {
	exit(0);
	return 0;
}

int bin_export(UNUSED(const char *name), char *const argv[], UNUSED(const char *opts), UNUSED(const int& func)) {
	char key[strlen(argv[1])+1];
	strcpy(key, argv[1]);
	char *val = strchr(key, '=');
	if (!val) return -1;
	*(val++) = 0;
	return setenv(key, val, 1);
}

int bin_unset(UNUSED(const char *name), char *const argv[], UNUSED(const char *opts), UNUSED(const int& func)) {
	return unsetenv(argv[1]);
}

int bin_fg(UNUSED(const char *name), char *const argv[], UNUSED(const char *opts), UNUSED(const int& func)) {
	size_t job_num = 1;
	if (argv[1])
		sscanf(argv[1], "%%%lu", &job_num);
	if (joblist.size()<job_num) {
		return -1;
	}
	--job_num;

	job &curr_job = joblist[job_num];
	if (kill(curr_job.pid.back(), SIGCONT) == -1) {
	}
	tcsetpgrp(0, joblist[job_num].pgid);
	if (joblist[job_num].waitpid(WUNTRACED) == 0) {
		joblist.erase(joblist.begin()+job_num);
	}
	tcsetpgrp(0, shell_pid);
	return 0;
}

int bin_jobs(UNUSED(const char *name), UNUSED(char *const argv[]), UNUSED(const char *opts), UNUSED(const int& func)) {
	size_t idx = 0;
	for(auto &curr_job: joblist) {
		++idx;
		printf("[%lu] ", idx);
		for(size_t i=0; i<curr_job.size(); i++) {
			for(auto &cmd: curr_job[i]) {
				printf(" %s", cmd.c_str());
			}
			if (i != curr_job.size()-1) {
				printf(" |");
			}
		}
		puts("");
	}
	return 0;
}

void init_builtins() {
	shell_pid = getpid();
	shell_pgid = getpgid(shell_pid);
	add_builtin("exit", bin_break, "", 0);
	add_builtin("export", bin_export, "", 0);
	add_builtin("unset", bin_unset, "", 0);
	add_builtin("fg", bin_fg, "", 0);
	add_builtin("jobs", bin_jobs, "", 0);
}

