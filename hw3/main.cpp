#include <cstdio>
#include <cstring>

#define BUF_SIZE 1024

using namespace std;

void output_prompt(const char* prompt) {
	printf(prompt);
}

void input_command(char *buf, int bufsize) {
	fgets(buf, bufsize, stdin);
	buf[strlen(buf)-1] = '\0';
}

int execute(char *cmd) {
	printf("execute: %s\n", cmd);
	return 0;
}

int main() {
	char prompt[] = "hw3sh > ";
	char buf[BUF_SIZE+1];
	while (1) {
		output_prompt(prompt);
		input_command(buf, BUF_SIZE);
		execute(buf);
	}
	return 0;
}
