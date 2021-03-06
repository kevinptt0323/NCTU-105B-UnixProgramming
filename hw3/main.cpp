#include <cstdio>
#include <cstring>
#include <vector>
#include <array>
#include <string>

#include <fcntl.h>
#include <unistd.h>
#include <signal.h>

#include "command.h"
#include "builtin.h"
#include "signal_handler.h"
#include "jobs.h"

#define BUF_SIZE 1024
#define error(STR, ...) \
	fprintf(stderr, "%s: " STR "\n", SHELL_NAME, ##__VA_ARGS__), \
	exit(EXIT_FAILURE)

const char* SHELL_NAME = "hw3sh";

using namespace std;

void output_prompt(const char* prompt) {
	printf(prompt);
}

char* input_command(char* buf, int bufsize) {
	char* ret = fgets(buf, bufsize, stdin);
	if (!ret) return NULL;
	buf[strlen(buf)-1] = '\0';
	if (!strlen(buf)) return NULL;
	return buf;
}

job parse(char* cmd) {
	job cmds;
	cmds.running = false;
	cmds.foreground = true;
	cmds.done = false;
	cmds.cmd = cmd;
	char *tmp_cmd, *ptr1, *ptr2;

	tmp_cmd = strtok_r(cmd, "|", &ptr1);
	do {
		if ((ptr2 = strchr(tmp_cmd, '&'))) {
			*ptr2 = ' ';
			cmds.foreground = false;
		}
		for(int i=0; tmp_cmd[i]; i++)
			if (tmp_cmd[i]>32) {
				cmds.emplace_back(tmp_cmd);
				break;
			}
	} while ((tmp_cmd = strtok_r(NULL, "|", &ptr1)));
	return cmds;
}

pid_t create_process(const command& argv0, int fd_in, int fd_out, vector<array<int,2>>& pipes_fd) {
	pid_t pid = fork();
	if (pid < 0) {
		error("%s", strerror(errno));
	} else if (pid == 0) {
		if (argv0.redirect_in != "") {
			int fd = open(argv0.redirect_in.c_str(), O_RDONLY);
			if (fd == -1) {
				error("%s: %s", strerror(errno), argv0.redirect_in.c_str());
			}
			dup2(fd, STDIN_FILENO);
			close(fd);
		} else if (fd_in != STDIN_FILENO)
			dup2(fd_in, STDIN_FILENO);
		if (argv0.redirect_out != "") {
			int fd = open(argv0.redirect_out.c_str(), O_WRONLY | O_CREAT, 0644);
			if (fd == -1) {
				error("%s: %s", strerror(errno), argv0.redirect_out.c_str());
			}
			dup2(fd, STDOUT_FILENO);
			close(fd);
		} else if (fd_out != STDOUT_FILENO)
			dup2(fd_out, STDOUT_FILENO);

		for(auto& pipe_fd: pipes_fd) {
			close(pipe_fd[0]);
			close(pipe_fd[1]);
		}

		char* argv[argv0.size()+1];
		for(size_t i=0; i<argv0.size(); i++) {
			argv[i] = (char*)argv0[i].c_str();
		}
		argv[argv0.size()] = NULL;

		if (execvp(argv[0], argv) == -1) {
			error("command not found: %s", argv[0]);
		}
	} else {
		return pid;
	}
	return 0;
}

int execute(const job& cmds) {
	if (cmds.size()==0) return 1;
	vector<array<int,2>> pipes_fd(cmds.size()-1);
	for(auto& pipe_fd: pipes_fd) {
		if (pipe(pipe_fd.data()) == -1) {
			//error("can't create pipe");
			error("%s", strerror(errno));
		}
	}

	job new_job = cmds;
	new_job.running = true;
	for(size_t i=0; i<cmds.size(); i++) {
		int fd_in  = i==0 ? STDIN_FILENO : pipes_fd[i-1][0];
		int fd_out = i==cmds.size()-1 ? STDOUT_FILENO : pipes_fd[i][1];
		auto &argv = cmds[i];
		if (builtins.find(argv[0]) != builtins.end()) {
			builtins[argv[0]].exec(argv);
			pid_t pid = 0;
			new_job.pid.emplace_back(pid);
		} else {
			pid_t pid = create_process(cmds[i], fd_in, fd_out, pipes_fd);
			new_job.pid.emplace_back(pid);
			if (i==0)
				new_job.pgid = pid;
			setpgid(pid, new_job.pgid);
		}
	}
	joblist.emplace_back(new_job);

	for(auto& pipe_fd: pipes_fd) {
		close(pipe_fd[0]);
		close(pipe_fd[1]);
	}

	job &curr_job = joblist.back();

	if (curr_job.foreground) {
		tcsetpgrp(0, curr_job.pgid);
		if (curr_job.waitpid(WUNTRACED) == 0) {
			joblist.done_pid(new_job.pid.back());
		}
		tcsetpgrp(0, shell_pgid);
	} else {
		joblist.print(joblist.size());
		if (curr_job.size()>1) {
			if (kill(curr_job.pid.back(), SIGSTOP) == -1)
				return errno;
		}
		//if (curr_job.waitpid(0) == 0) {
		//	curr_job.running = false;
		//}
	}

	//printf("execute: %s\n", cmd);
	return 0;
}

int main() {
	char prompt[] = "hw3sh> ";
	char buf[BUF_SIZE+1];

	init_signal_handler();
	init_builtins();
	while (1) {
		output_prompt(prompt);
		if (input_command(buf, BUF_SIZE))
			execute(parse(buf));
	}
	return 0;
}
