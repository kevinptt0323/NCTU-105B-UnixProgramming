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
	int waitpid(int);
	bool running;
	bool foreground;
	string cmd;
};

struct jobs : vector<job> {
};

#endif
