#ifndef __CONNECTION_H__
#define __CONNECTION_H__

#include <iostream>
#include <iomanip>
#include <sstream>
using namespace std;

union IPv4 {
	unsigned char digit[4];
	unsigned raw;
};

ostream& operator<<(ostream& out, const IPv4& ip) {
	ostringstream oss;
	oss << (int)ip.digit[0] << "."
		<< (int)ip.digit[1] << "."
		<< (int)ip.digit[2] << "."
		<< (int)ip.digit[3];
	return out << oss.str();
}

union IPv6 {
	unsigned short digit[8];
	__uint128_t raw;
};

ostream& operator<<(ostream& out, const IPv6& ip) {
	ostringstream oss;
	bool first = true;
	int cnt = 0;
	bool double_colon = false;
	for(int i=7; i>=0; i--) {
		if (double_colon || ip.digit[i]) {
			if (first && cnt>=2) {
				oss << ":";
				double_colon = true;
			}
			if (cnt>=1) {
				oss << ":";
				if (!first) double_colon = true;
			}
			oss << hex << ip.digit[i];
			if (i!=0)
				oss << ":";
			first = false;
			cnt = 0;
		} else cnt++;
	}
	if (cnt) {
		oss << (first ? "::" : ":");
	}
	return out << oss.str();
}

struct Address {
	union {
		IPv4 v4;
		IPv6 v6;
	} ip;
	int type;
	unsigned short port;
};

ostream& operator<<(ostream& out, const Address& addr) {
	ostringstream oss;
	switch(addr.type) {
		case 4:
			oss << addr.ip.v4;
			break;
		case 6:
			oss << addr.ip.v6;
			break;
	}
	oss << ":";
	if (addr.port==0) oss << "*";
	else oss << addr.port;
	return out << oss.str();
}

struct Connection {
	Address local, remote;
	int inode;
};

#endif
