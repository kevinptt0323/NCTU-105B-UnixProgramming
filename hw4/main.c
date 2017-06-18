#include "othello.h"

static int width;
static int height;
static int cx = 3;
static int cy = 3;

const int dx[] = {1, 0, -1, 0, 1, 1, -1, -1};
const int dy[] = {0, 1, 0, -1, 1, -1, -1, 1};

void update_board(int _cx, int _cy, int player) {
	if (board[_cy][_cx]) return;
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
}

int
main(int argc, char* argv[])
{	
	initscr();			// start curses mode 
	getmaxyx(stdscr, height, width);// get screen size

	cbreak();			// disable buffering
					// - use raw() to disable Ctrl-Z and Ctrl-C as well,
	halfdelay(1);			// non-blocking getch after n * 1/10 seconds
	noecho();			// disable echo
	keypad(stdscr, TRUE);		// enable function keys and arrow keys
	curs_set(0);			// hide the cursor

	init_colors();

restart:
	clear();
	cx = cy = 3;
	init_board();
	draw_board();
	draw_cursor(cx, cy, 1);
	draw_score();
	refresh();

	attron(A_BOLD);
	move(height-1, 0);	printw("Arrow keys: move; Space: put GREEN; Return: put PURPLE; R: reset; Q: quit");
	attroff(A_BOLD);

	while(true) {			// main loop
		int ch = getch();
		int moved = 0;

		switch(ch) {
		case ' ':
			update_board(cx, cy, PLAYER1);
			draw_score();
			break;
		case 0x0d:
		case 0x0a:
		case KEY_ENTER:
			update_board(cx, cy, PLAYER2);
			draw_score();
			break;
		case 'q':
		case 'Q':
			goto quit;
			break;
		case 'r':
		case 'R':
			goto restart;
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

		if(moved) {
			refresh();
			moved = 0;
		}

		napms(1);		// sleep for 1ms
	}

quit:
	endwin();			// end curses mode

	return 0;
}
