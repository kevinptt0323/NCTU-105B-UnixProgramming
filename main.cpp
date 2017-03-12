#include <cstdio>
#include <cstring>
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

const char* TCP_FILE = "/proc/net/tcp";
const char* UDP_FILE = "/proc/net/udp";

map<int,string> inode2pid;

vector<Connection> parse(const char* filename) {
	FILE* fin = fopen(filename, "r");
	char* buffer = new char[1024];
	vector<Connection> ret;
	fgets(buffer, 1023, fin);
	while (fgets(buffer, 1023, fin)) {
		Connection conn;
		sscanf(buffer, "%*d: %x:%hx %x:%hx %*s %*s %*s %*s %*s %*s %d", &conn.local.ip.raw, &conn.local.port, &conn.remote.ip.raw, &conn.remote.port, &conn.inode);
		ret.emplace_back(conn);
	}
	fclose(fin);
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
	oss << setw(23) << left << conn.local << " " << setw(23) << left << conn.remote;
	if (inode2pid.find(conn.inode)!=inode2pid.end())
		oss << " " << setw(30) << left << inode2pid[conn.inode];
	else
		oss << " " << setw(30) << left << "-";
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
}

void show_udp() {
	cout << "List of UDP connections:" << endl;
	auto udp_conn = parse(UDP_FILE);
	for(const auto& conn: udp_conn) {
		cout << setw(5) << left << "udp " << " " << conn << endl;
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
