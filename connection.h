#ifndef __CONNECTION_H__
#define __CONNECTION_H__

#include <iostream>
#include <sstream>
using namespace std;

union IP {
	unsigned char digit[4];
	unsigned raw;
};

ostream& operator<<(ostream& out, const IP& ip) {
	ostringstream oss;
	oss << (int)ip.digit[0] << "."
		<< (int)ip.digit[1] << "."
		<< (int)ip.digit[2] << "."
		<< (int)ip.digit[3];
	return out << oss.str();
}

struct Address {
	IP ip;
	unsigned short port;
};

ostream& operator<<(ostream& out, const Address& addr) {
	ostringstream oss;
	oss << addr.ip << ":";
	if (addr.port==0) oss << "*";
	else oss << addr.port;
	return out << oss.str();
}

struct Connection {
	Address local, remote;
	int inode;
};

#endif
