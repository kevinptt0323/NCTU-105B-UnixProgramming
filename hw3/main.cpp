#include <cstdio>
#include <cstring>
#include <vector>
#include <string>

#define BUF_SIZE 1024

using namespace std;

void output_prompt(const char* prompt) {
	printf(prompt);
}

void input_command(char* buf, int bufsize) {
	fgets(buf, bufsize, stdin);
	buf[strlen(buf)-1] = '\0';
}

vector<vector<string>> parse(char* cmd) {
	vector<vector<string>> cmds;
	char *tmp_cmd, *ptr1;
	tmp_cmd = strtok_r(cmd, "|", &ptr1);
	do {
		cmds.resize(cmds.size()+1);
		auto& vec = cmds.back();
		char *argv, *ptr2;
		argv = strtok_r(tmp_cmd, " ", &ptr2);
		do vec.emplace_back(argv);
		while ((argv = strtok_r(NULL, " ", &ptr2)));
	} while ((tmp_cmd = strtok_r(NULL, "|", &ptr1)));

	return cmds;
}

int execute(const vector<vector<string>>& cmds) {
	for(const auto& cmd: cmds) {
		for(const auto& argv: cmd) {
			printf("%s ", argv.c_str());
		}
		puts("");
	}
	//printf("execute: %s\n", cmd);
	return 0;
}

int main() {
	char prompt[] = "hw3sh > ";
	char buf[BUF_SIZE+1];
	while (1) {
		output_prompt(prompt);
		input_command(buf, BUF_SIZE);
		execute(parse(buf));
	}
	return 0;
}
