#include "builtin.h"
#include <string>
#include <unordered_map>
#include <vector>

#define add_builtin(name, ...) \
	builtins[name] = builtin(name, __VA_ARGS__)

#define UNUSED(var) var __attribute__((unused))

using std::string;
using std::unordered_map;
using std::vector;

unordered_map<string, builtin> builtins;

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

void init_builtins() {
	add_builtin("exit", bin_break, "", 0);
}

