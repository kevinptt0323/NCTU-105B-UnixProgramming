#include <cstdio>
#include <cstring>
#include <vector>
#include <array>
#include <string>

#include <fcntl.h>
#include <unistd.h>

#include "command.h"

#define BUF_SIZE 1024
#define error(STR, ...) \
	fprintf(stderr, "%s: " STR "\n", SHELL_NAME, ##__VA_ARGS__), \
	exit(EXIT_FAILURE)

const char* SHELL_NAME = "hw3sh";

using namespace std;

void output_prompt(const char* prompt) {
	printf(prompt);
}

void input_command(char* buf, int bufsize) {
	fgets(buf, bufsize, stdin);
	buf[strlen(buf)-1] = '\0';
}

vector<command> parse(char* cmd) {
	vector<command> cmds;
	char *tmp_cmd, *ptr1;
	tmp_cmd = strtok_r(cmd, "|", &ptr1);
	do {
		cmds.emplace_back(tmp_cmd);
	} while ((tmp_cmd = strtok_r(NULL, "|", &ptr1)));

	return cmds;
}

pid_t create_process(const command& argv0, int fd_in, int fd_out, vector<array<int,2>>& pipes_fd) {
	pid_t pid = fork();
	if (pid < 0) {
		error("can't fork");
	} else if (pid == 0) {
		if (argv0.redirect_in != "") {
			int fd = open(argv0.redirect_in.c_str(), O_RDONLY);
			if (fd == -1) {
				error("no such file or directory: %s", argv0.redirect_in.c_str());
			}
			dup2(fd, STDIN_FILENO);
			close(fd);
		} else if (fd_in != STDIN_FILENO)
			dup2(fd_in, STDIN_FILENO);
		if (argv0.redirect_out != "") {
			int fd = open(argv0.redirect_out.c_str(), O_WRONLY | O_CREAT, 0644);
			if (fd == -1) {
				error("can't write such file: %s", argv0.redirect_out.c_str());
			}
			dup2(fd, STDOUT_FILENO);
			close(fd);
		} else if (fd_out != STDOUT_FILENO)
			dup2(fd_out, STDOUT_FILENO);

		for(auto& pipe_fd: pipes_fd) {
			close(pipe_fd[0]);
			close(pipe_fd[1]);
		}

		char** argv = new char*[argv0.size()+1];
		for(size_t i=0; i<argv0.size(); i++) {
			argv[i] = new char[argv0[i].size()+1];
			strcpy(argv[i], argv0[i].c_str());
		}
		argv[argv0.size()] = NULL;

		if (execvp(argv[0], argv) == -1) {
			error("command not found: %s", argv[0]);
		}
	} else {
		return pid;
	}
}

int execute(const vector<command>& cmds) {
	vector<array<int,2>> pipes_fd(cmds.size()-1);
	for(auto& pipe_fd: pipes_fd) {
		if (pipe(pipe_fd.data()) == -1) {
			error("can't create pipe");
		}
	}

	for(size_t i=0; i<cmds.size(); i++) {
		int fd_in  = i==0 ? STDIN_FILENO : pipes_fd[i-1][0];
		int fd_out = i==cmds.size()-1 ? STDOUT_FILENO : pipes_fd[i][1];
		create_process(cmds[i], fd_in, fd_out, pipes_fd);
	}

	for(auto& pipe_fd: pipes_fd) {
		close(pipe_fd[0]);
		close(pipe_fd[1]);
	}

	for(size_t i=0; i<cmds.size(); i++) {
		wait(NULL);
	}

	//printf("execute: %s\n", cmd);
	return 0;
}

int main() {
	char prompt[] = "hw3sh> ";
	char buf[BUF_SIZE+1];
	while (1) {
		output_prompt(prompt);
		input_command(buf, BUF_SIZE);
		execute(parse(buf));
	}
	return 0;
}
