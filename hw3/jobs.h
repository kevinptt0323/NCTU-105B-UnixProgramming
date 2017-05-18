#ifndef __JOBS_H__
#define __JOBS_H__

#include <list>
#include <vector>
#include <fcntl.h>

#include "command.h"


using std::vector;

struct job : vector<command> {
	vector<pid_t> pid;
	pid_t pgid;
	bool running;
	bool foreground;
	bool done;
	string cmd;

	int waitpid(int);
	void print(FILE* fout=stdout) const;
};

struct jobs : vector<job> {
	void done_pid(pid_t);
	void done_job(size_t);
	void done_job(jobs::iterator);
	void print(size_t, FILE* fout=stdout) const;
};

#endif
