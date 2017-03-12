#include <cstdio>
#include <cstring>
#include <cctype>
#include <dirent.h>
#include <sys/stat.h>

#include <iostream>
#include <sstream>
#include <iomanip>
#include <string>
#include <vector>
#include <map>
#include "connection.h"
using namespace std;

const char* TCP_FILE  = "/proc/net/tcp";
const char* TCP6_FILE = "/proc/net/tcp6";
const char* UDP_FILE  = "/proc/net/udp";
const char* UDP6_FILE = "/proc/net/udp6";

map<int,string> inode2pid;

void transform_endian(char* s) {
	for(int i=0; i<32; i+=8) {
		swap(s[i], s[i+6]);
		swap(s[i+1], s[i+7]);
		swap(s[i+2], s[i+4]);
		swap(s[i+3], s[i+5]);
	}
}

__uint128_t atolll(const char* s) {
	__uint128_t v = 0;
	for(int i=0; (s[i]>='0' && s[i]<='9') || (tolower(s[i])>='a'&&tolower(s[i])<='f'); i++)
		v = (v<<4) + (s[i]>='0'&&s[i]<='9' ? s[i]-48 : tolower(s[i])-'a'+10);
	return v;
}

vector<Connection> parse(const char* filename, const int& type = 4) {
	FILE* fin = fopen(filename, "r");
	char* buffer = new char[1024];
	char* local_addr = new char[1024];
	char* remote_addr = new char[1024];
	vector<Connection> ret;
	fgets(buffer, 1023, fin);
	while (fgets(buffer, 1023, fin)) {
		Connection conn;
		sscanf(buffer, "%*d: %s %s %*s %*s %*s %*s %*s %*s %d", local_addr, remote_addr, &conn.inode);
		if (type==4) {
			sscanf(local_addr, "%x:%hx", &conn.local.ip.v4.raw, &conn.local.port);
			sscanf(remote_addr, "%x:%hx", &conn.remote.ip.v4.raw, &conn.remote.port);
		}
		else if (type==6) {
			sscanf(local_addr, "%*x:%hx", &conn.local.port);
			sscanf(remote_addr, "%*x:%hx", &conn.remote.port);
			transform_endian(local_addr);
			transform_endian(remote_addr);
			conn.local.ip.v6.raw = atolll(local_addr);
			conn.remote.ip.v6.raw = atolll(remote_addr);
		}
		conn.local.type = type;
		conn.remote.type = type;
		ret.emplace_back(conn);
	}
	fclose(fin);
	delete[] buffer;
	delete[] local_addr;
	delete[] remote_addr;
	return ret;
}

vector<string> ls(string dir) {
	if (dir.back()!='/') dir += "/";
	vector<string> ret;
	DIR* dirp = opendir(dir.c_str());
	if (dirp == NULL) return ret;
	dirent *direntp;
	while ((direntp = readdir(dirp)) != NULL){
		if (strcmp(direntp->d_name, ".")==0 || strcmp(direntp->d_name, "..")==0) continue;
		ret.emplace_back(dir + direntp->d_name);
	}
	closedir(dirp);
	return ret;
}

vector<string> getfd() {
	auto files = ls("/proc/");
	vector<string> fd;
	for(const auto& dir: files) {
		auto fd_tmp = ls(dir + "/fd/");
		fd.insert(fd.end(), fd_tmp.begin(), fd_tmp.end());
	}
	return fd;
}

ostream& operator<<(ostream& out, const Connection& conn) {
	ostringstream oss;
	oss << setw(23) << left << conn.local << " "
		<< setw(23) << left << conn.remote << " ";
	if (inode2pid.find(conn.inode)!=inode2pid.end())
		oss << setw(30) << left << inode2pid[conn.inode];
	else
		oss << setw(30) << left << "-";
	return out << oss.str();
}

void show_tcp() {
	cout << "List of TCP connections:" << endl;
	cout << setw(5)  << left << "Proto" << " "
		 << setw(23) << left << "Local Address" << " "
		 << setw(23) << left << "Remote Address" << " "
		 << setw(30) << left << "PID/Program name and arguments"
		 << endl;
	auto tcp_conn = parse(TCP_FILE);
	for(const auto& conn: tcp_conn) {
		cout << setw(5) << left << "tcp" << " " << conn << endl;
	}
	auto tcp6_conn = parse(TCP6_FILE, 6);
	for(const auto& conn: tcp6_conn) {
		cout << setw(5) << left << "tcp6" << " " << conn << endl;
	}
}

void show_udp() {
	cout << "List of UDP connections:" << endl;
	cout << setw(5)  << left << "Proto" << " "
		 << setw(23) << left << "Local Address" << " "
		 << setw(23) << left << "Remote Address" << " "
		 << setw(30) << left << "PID/Program name and arguments"
		 << endl;
	auto udp_conn = parse(UDP_FILE);
	for(const auto& conn: udp_conn) {
		cout << setw(5) << left << "udp " << " " << conn << endl;
	}
		auto udp6_conn = parse(UDP6_FILE, 6);
	for(const auto& conn: udp6_conn) {
		cout << setw(5) << left << "udp6" << " " << conn << endl;
	}
}

int main() {
	{
		inode2pid.clear();
		auto fd = getfd();
		struct stat stat_buf;
		for(const auto& dir: fd) {
			stat(dir.c_str(), &stat_buf);
			if (S_ISSOCK(stat_buf.st_mode))
				inode2pid[stat_buf.st_ino] = dir;
		}
	}
	show_tcp();
	cout << endl;
	show_udp();
	return 0;
}
