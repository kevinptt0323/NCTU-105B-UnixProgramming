#include <cstdio>
#include <iostream>
#include <vector>
#include "connection.h"
using namespace std;

const char* TCP_FILE = "/proc/net/tcp";
const char* UDP_FILE = "/proc/net/udp";

vector<Connection> parse(const char* filename) {
	FILE* fin = fopen(filename, "r");
	char* buffer = new char[1024];
	vector<Connection> ret;
	fgets(buffer, 1023, fin);
	while (fgets(buffer, 1023, fin)) {
		Connection conn;
		sscanf(buffer, "%*d: %x:%hx %x:%hx", &conn.local.ip.raw, &conn.local.port, &conn.remote.ip.raw, &conn.remote.port);
		ret.emplace_back(conn);
	}
	fclose(fin);
	return ret;
}

int main() {
	auto tcp_conn = parse(TCP_FILE);
	auto udp_conn = parse(UDP_FILE);
	for(const auto& conn: tcp_conn) {
		cout << conn.local << "\t" << conn.remote << endl;
	}

	for(const auto& conn: udp_conn) {
		cout << conn.local << "\t" << conn.remote << endl;
	}
	return 0;
}
