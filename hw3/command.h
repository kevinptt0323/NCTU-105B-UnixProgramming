#ifndef __COMMAND_H__
#define __COMMAND_H__

#include <vector>
#include <string>
using std::vector;
using std::string;

class command : public vector<string> {
public:
	string redirect_in, redirect_out;
	command();
	command(const char*);
	command(const string&);
};

#endif
