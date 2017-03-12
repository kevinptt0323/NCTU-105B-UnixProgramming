#ifndef __CONNECTION_H__
#define __CONNECTION_H__

#include <iostream>
using namespace std;

union IP {
	unsigned char digit[4];
	unsigned raw;
};

ostream& operator<<(ostream& out, const IP& ip) {
	out << (int)ip.digit[0] << "."
		<< (int)ip.digit[1] << "."
		<< (int)ip.digit[2] << "."
		<< (int)ip.digit[3];
	return out;
}

struct Address {
	IP ip;
	unsigned short port;
};

ostream& operator<<(ostream& out, const Address& addr) {
	out << addr.ip << ":";
	if (addr.port==0) out << "*";
	else out << addr.port;
	return out;
}

struct Connection {
	Address local, remote;
};

#endif
