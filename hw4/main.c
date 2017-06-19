#include "othello.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdbool.h>

static int width;
static int height;
static int cx = 3;
static int cy = 3;

int start_server(char* arg) {
	int server_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	struct sockaddr_in server, client;

	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(atoi(arg));

	if (bind(server_fd, (struct sockaddr *)&server, sizeof(struct sockaddr_in)) == -1) {
		fprintf(stderr, "Could not binding socket\n");
		return -1;
	}

	if (listen(server_fd, 128) == -1) {
		fprintf(stderr, "Could not listen on socket\n");
		return -1;
	}

	fprintf(stderr, "listen on %d\n", atoi(arg));

	char buf[2048];
	int buf_sz;
	while(1) {
		socklen_t client_len = sizeof(client);
		int client_fd = accept(server_fd, (struct sockaddr *) &client, &client_len);

		if (client_fd < 0) {
			fprintf(stderr, "Could not establish new connection\n");
			return -1;
		}

		return client_fd;

		while(1) {
			if ((buf_sz=recv(client_fd, buf, 2047, 0)) > 0) {
				buf[buf_sz] = 0;
				printf("recv: %s\n", buf);
			}
		}
	}
}
int start_client(char* arg) {
	int server_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	struct sockaddr_in server;
	char ip[20], port[10];
	for(int i=0, i2=-1; arg[i]; ++i) {
		if (arg[i]!=':') {
			if (i2==-1) ip[i] = arg[i], ip[i+1] = 0;
			else port[i2++] = arg[i], port[i2] = 0;
		} else {
			i2 = 0;
		}
	}
	if (strcmp(ip, "localhost") == 0) strcpy(ip, "127.0.0.1");

	server.sin_family = AF_INET;
	server.sin_addr.s_addr = inet_addr(ip);
	server.sin_port = htons(atoi(port));

	if (connect(server_fd, (struct sockaddr *)&server, sizeof(struct sockaddr_in)) == -1) {
		fprintf(stderr, "Could not connect to %s\n", arg);
		return -1;
	}

	return server_fd;
}

const int dx[] = {1, 0, -1, 0, 1, 1, -1, -1};
const int dy[] = {0, 1, 0, -1, 1, -1, -1, 1};

bool validate(int _cx, int _cy, int player) {
	if (board[_cy][_cx]) return false;
	for(int i=0; i<8; i++) {
		int count = 0;
		for (int x=_cx+dx[i], y=_cy+dy[i]; x>=0 && x<BOARDSZ && y>=0 && y<BOARDSZ; x+=dx[i], y+=dy[i]) {
			if (board[y][x] == 0) break;
			else if (board[y][x] == player) {
				if (count>0) return true;
				break;
			} else count++;
		}
	}
	return false;
}

bool update_board(int _cx, int _cy, int player) {
	if (!validate(_cx, _cy, player)) return false;
	for(int i=0; i<8; i++) {
		for (int x=_cx+dx[i], y=_cy+dy[i]; x>=0 && x<BOARDSZ && y>=0 && y<BOARDSZ; x+=dx[i], y+=dy[i]) {
			if (board[y][x] == 0) break;
			else if (board[y][x] == player) {
				for (int x2=_cx, y2=_cy; x2!=x || y2!=y; x2+=dx[i], y2+=dy[i]) {
					board[y2][x2] = player;
					draw_cursor(x2, y2, 1);
				}
				break;
			}
		}
	}
	draw_score();
	draw_board();
	return true;
}

int check_fd(int fd) {
	fd_set read_fds;
	FD_ZERO(&read_fds);
	FD_SET(fd, &read_fds);

	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 0;

	if (select(fd+1, &read_fds, NULL, NULL, &tv) == -1) {
		fprintf(stderr, "error select\n");
	}
	return FD_ISSET(fd, &read_fds);
}

int
main(int argc, char* argv[])
{	
	int fd = 0, player;
	if (argc!=3) return 0;
	if (strcmp(argv[1], "-s") == 0) {
		fd = start_server(argv[2]);
		player = PLAYER1;
	} else if (strcmp(argv[1], "-c") == 0) {
		fd = start_client(argv[2]);
		player = PLAYER2;
	}
	if (fd<0) return 0;

	int current_player = PLAYER1;
	char buf[64];

	initscr();			// start curses mode 
	getmaxyx(stdscr, height, width);// get screen size

	cbreak();			// disable buffering
					// - use raw() to disable Ctrl-Z and Ctrl-C as well,
	halfdelay(1);			// non-blocking getch after n * 1/10 seconds
	noecho();			// disable echo
	keypad(stdscr, TRUE);		// enable function keys and arrow keys
	curs_set(0);			// hide the cursor

	init_colors();

	clear();
	cx = cy = 3;
	init_board();
	draw_board();
	draw_cursor(cx, cy, 1);
	draw_score();

	attron(A_BOLD);
	move(0, 0);	printw("Player #%d %s", (player==PLAYER1)?1:2, current_player==player?"your turn!":"please wait...");
	attroff(A_BOLD);
	attron(A_BOLD);
	move(height-1, 0);	printw("Arrow keys: move; Space/Return: put; Q: quit");
	attroff(A_BOLD);

	refresh();

	while(true) {			// main loop
		int moved = 0;
		if (check_fd(STDIN_FILENO)) {
			int ch = getch();

			switch(ch) {
			case ' ':
			case 0x0d:
			case 0x0a:
			case KEY_ENTER:
				if (current_player==player) {
					if (update_board(cx, cy, player)) {
						sprintf(buf, "c %d %d %d", cx, cy, player);
						send(fd, buf, strlen(buf), 0);
						current_player = -current_player;
					}
				}
				moved++;
				break;
			case 'q':
			case 'Q':
				send(fd, "q", 1, 0);
				goto quit;
				break;
			case 'k':
			case KEY_UP:
				draw_cursor(cx, cy, 0);
				cy = (cy-1+BOARDSZ) % BOARDSZ;
				draw_cursor(cx, cy, 1);
				moved++;
				break;
			case 'j':
			case KEY_DOWN:
				draw_cursor(cx, cy, 0);
				cy = (cy+1) % BOARDSZ;
				draw_cursor(cx, cy, 1);
				moved++;
				break;
			case 'h':
			case KEY_LEFT:
				draw_cursor(cx, cy, 0);
				cx = (cx-1+BOARDSZ) % BOARDSZ;
				draw_cursor(cx, cy, 1);
				moved++;
				break;
			case 'l':
			case KEY_RIGHT:
				draw_cursor(cx, cy, 0);
				cx = (cx+1) % BOARDSZ;
				draw_cursor(cx, cy, 1);
				moved++;
				break;
			}
		} else if (check_fd(fd)) {
			int cx2, cy2, player2, len;
			len = recv(fd, buf, 63, 0);
			buf[len] = 0;
			switch(buf[0]) {
			case 'c':
				sscanf(buf, "%*s %d %d %d", &cx2, &cy2, &player2);
				update_board(cx2, cy2, player2);
				current_player = -current_player;
				moved++;
				break;
			case 'q':
				goto quit;
			}
		}

		if(moved) {
			refresh();
			moved = 0;
		}

		napms(1);		// sleep for 1ms
	}

quit:
	endwin();			// end curses mode
	close(fd);

	return 0;
}
