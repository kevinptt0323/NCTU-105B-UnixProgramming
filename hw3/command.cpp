#include "command.h"
#include <cstring>
#include <string>
#include <glob.h>
using std::string;

command::command() {
	redirect_in = redirect_out = "";
}

command::command(const char* cmd) {
	char *cmd1 = new char[strlen(cmd)+1];
	strcpy(cmd1, cmd);
	char *argv, *ptr;
	argv = strtok_r(cmd1, " ", &ptr);
	do {
		if (strcmp(argv, "<")==0)
			redirect_in = strtok_r(NULL, " ", &ptr);
		else if (strcmp(argv, ">")==0)
			redirect_out = strtok_r(NULL, " ", &ptr);
		else if (strchr(argv, '*') || strchr(argv, '?')) {
			glob_t buf;
			glob(argv, GLOB_TILDE, NULL, &buf);
			for(size_t i=0; i<buf.gl_pathc; i++) {
				emplace_back(buf.gl_pathv[i]);
			}
			if (!buf.gl_pathc) {
				emplace_back(argv);
			}
			globfree(&buf);
		} else {
			emplace_back(argv);
		}
	} while ((argv = strtok_r(NULL, " ", &ptr)));
}

command::command(const string& cmd) {
	command(cmd.c_str());
}
