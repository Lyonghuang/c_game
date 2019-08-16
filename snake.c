#include <time.h>
#include <ncurses.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define FRAME_H 15
#define FRAME_W 30
#define UP 119
#define DOWN 115
#define LEFT 97
#define RIGHT 100
#define EXIT 101
#define PAUSE 112

int score = 0;
int bodyLength;
int direction;
int gameOver;
int gamePause;

struct point {
	int x,y;
};
struct point body [100];
struct point food;

void init();
void printFrame();
void initGame();
void printSnake(int display);
int  updateFood();
void updateBody();
void *updateGame(void *args);
void *getCommand(void *args);

int main() {
	pthread_t commThread, updateThread;
	
	init();
	initGame();
	printFrame();
	printSnake(1);
	
	pthread_create(&updateThread, NULL, updateGame, NULL);
	pthread_create(&commThread, NULL, getCommand, NULL);
	
	int ch = getch();
	while (!gameOver) {
		//ch = getch();
	}
	getch();
	endwin();
	pthread_exit(NULL);
}

void init() {
	srand((int)time(0));//设置随机数种子
	initscr();//获取终端屏幕信息
	cbreak();//禁用行缓冲
	noecho();//禁用输入回显
	clear();//清屏
	curs_set(0);//禁用光标
}

void initGame() {
	score = 0;
	bodyLength = 3;
	direction = RIGHT;
	gameOver = 0;
	gamePause = 0;

	body[0].x = FRAME_H / 2;
	body[0].y = FRAME_W / 2;
	
	body[1].x = body[0].x;
	body[1].y = body[0].y - 1;
	
	body[2].x = body[0].x;
	body[2].y = body[0].y - 2;

	for (int i=3; i<100; i++) {
		body[i].x = 0;
		body[i].y = 0;
	}
	
	while (!updateFood());
	//mvprintw(food.x, food.y, "*");
}

void printFrame(){
	mvprintw(0, 0, "+");
	mvprintw(0, FRAME_W+1, "+");
	mvprintw(FRAME_H+1, 0, "+");
	mvprintw(FRAME_H+1, FRAME_W+1, "+");

	for (int i=1; i<=FRAME_W; i++) {
		mvprintw(0, i, "-");
		mvprintw(FRAME_H+1, i, "-");
	}
	
	for (int i=1; i<=FRAME_H; i++) {
		mvprintw(i, 0, "|");
		mvprintw(i, FRAME_W+1, "|");
	}

	mvprintw(5, FRAME_W + 4, "score:%d", score);
	mvprintw(food.x, food.y, "*");
}

void printSnake(int display) {
	int x, y;
	
	x = body[0].x;
	y = body[0].y;
	if (display){
		mvprintw(x, y, "@");
	}
	else {
		mvprintw(x, y, " ");
	}

	for (int i=1; i<bodyLength; i++) {
		x = body[i].x;
		y = body[i].y;
		if (display){
		       mvprintw(x, y, "#");
		}
		else {
			mvprintw(x, y, " ");
		}
	}
	refresh();
}

int updateFood() {
	int x = rand() % FRAME_H + 1;
	int y = rand() % FRAME_W + 1;
	for (int i=0; i<bodyLength; i++) {
		if (x == body[i].x && y == body[i].y) {
			return 0;
		}
	}
	mvprintw(food.x, food.y, " ");
	food.x = x;
	food.y = y;
	mvprintw(food.x, food.y, " ");
	return 1;
}

void updateBody() {
	int nextX, nextY;

	printSnake(0);
	switch(direction) {
		case UP:
			nextX = body[0].x - 1;
			nextY = body[0].y;
			break;
		case DOWN:
			nextX = body[0].x + 1;
			nextY = body[0].y;
			break;
		case LEFT:
			nextX = body[0].x;
			nextY = body[0].y - 1;
			break;
		case RIGHT:
			nextX = body[0].x;
			nextY = body[0].y + 1;
			break;
	}
	
	for (int i=bodyLength-1; i>0; i--) {
		body[i].x = body[i-1].x;
		body[i].y = body[i-1].y;
	}
	body[0].x = nextX;
	body[0].y = nextY;
	printSnake(1);
}

void *getCommand(void *args) {
	int ch;
	while (!gameOver) {
		ch = getch();
		switch(ch) {
			case RIGHT:
				if (direction != LEFT) {
					direction = RIGHT;
				}
				break;
			case LEFT:
				if (direction != RIGHT ){
					direction = LEFT;
				}
				break;
			case UP:
				if (direction != DOWN) {
					direction = UP;
				}
				break;
			case DOWN:
				if (direction != UP) {
					direction = DOWN;
				}
				break;
			case EXIT:
				gameOver = 1;
				break;
			case PAUSE:
				if (gamePause) {
					gamePause = 0; 
				}
				else {
					gamePause = 1;
				}
				break;
		}
	}
}

void *updateGame(void *args) {
	while (!gameOver) {
		sleep(1);
		if (!gamePause) {
			updateBody();
		}
	}
}
