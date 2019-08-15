#include <ncurses.h>
#include <time.h>
#include <stdlib.h>

#define FRAME_H 20
#define FRAME_W 10

void init();
void welcome();
void printFrame();


int main() {
	init();
	welcome();
	printFrame();
	getch();
	return 0;
}

void init(){
	srand((int)time(0));
	initscr();
	cbreak();
	noecho();
	clear();
	return;
}

void welcome() {
	clear();
	mvprintw(0, 0, "welcome!\n");
	mvprintw(3, 0, "Press 's' to start\n");
	int ch = getch();
	while (ch != 's') {
		ch = getch();
	}
	return;
}

void printFrame() {
	clear();
	mvprintw(0, 0, "+");
	mvprintw(FRAME_H+1, 0, "+");
	mvprintw(0, FRAME_W+1, "+");
	mvprintw(FRAME_H+1, FRAME_W+1, "+");
	
	for (int i=1; i<FRAME_W; i++) {
		mvprintw(0, i, "-");
		mvprintw(FRAME_H+1, i, "-");
	}
	
	for (int i=1; i<FRAME_H; i++) {
		mvprintw(i, 0, "|");
		mvprintw(i, FRAME_W+1, "|");
	}

	mvprintw(1, FRAME_W + 4, "next");
	mvprintw(FRAME_H - 4, FRAME_W + 4, "score:");
	return;
}
