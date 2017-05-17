#ifndef __BUILTIN_H__
#define __BUILTIN_H__

#include <cstring>
#include <string>
#include <unordered_map>
#include <vector>

#include "jobs.h"

using std::string;
using std::unordered_map;
using std::vector;

typedef int (*HandlerFunc)(const char*, char *const[], const char*, const int&);

struct builtin {
	string name;
	HandlerFunc handler;
	string opts;
	int funcid;

	builtin();
	builtin(const char*, HandlerFunc, const char*, int);
	~builtin();
	int exec(char *const[]);
	int exec(const vector<string>&);
};

extern unordered_map<string, builtin> builtins;
extern jobs joblist;
extern int shell_pid, shell_pgid;

void init_builtins();

#endif
