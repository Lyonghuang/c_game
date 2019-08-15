#include <ncurses.h>
#include <time.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define FRAME_H 20//方框高
#define FRAME_W 10//方框宽
#define ROTATE 119//旋转命令
#define LEFT 97//左移
#define RIGHT 100//右移
#define DOWN 115//加速下落
#define PAUSE 112//暂停
#define EXIT 101//结束游戏

struct point{//方块的小正方形的位置
	int x,y;
};

struct block{//一个方块，由四个小正方形构成
	struct point p[4];
};

struct block_t{//用于当前方块和下一方块的结构体，id和shape分别表示方块数组的两维下标
	int id;
	int shape;
};

int room[FRAME_H+1][FRAME_W+1];
/*0: | 1:O 2:L 3:_L 4:T 5:Z 6:_Z*/
struct block blocks[7][4];//一共有7种方块，每种方块通过旋转有四种形态
struct block_t curBlock, nextBlock;//当前方块和下一个方块
int score = 0;//得分
struct point curPosition;//方块的当前位置
int gameOver;//游戏结束参数
int gamePause;//游戏暂停参数
int speed;

void init();//初始化窗口和随机数种子等操作
void welcome();//欢迎界面
void printFrame();//绘制方框
void initGame();//初始化游戏数据
struct point makePoint(int x, int y);//构造点的一个函数
void setBlocks();//初始化每种方块每种形状的数据
void printBlock(int x, int y, struct block target, int display);//在指定位置绘制或清除指定方块，display为1表示绘制，为0表示清除方块
void printCurBlock();//绘制当前方块
void *getCommand(void *args);//监听游戏控制的函数
void doRotate();//旋转处理
void doLeft();//左移处理
void doRight();//右移处理
void doPause();//暂停处理
void doExit();//退出游戏处理
void *updateGame(void *args);//定时更新游戏的函数，主要是每秒将方块下落一个位置
int ifBlockDrop();//判断当前方块是否可以继续下落
void newBlock();//一个方块完成，新产生一个方块
void doBlockDrop();//将方块下落一格

int main() {
	pthread_t commThread, updateThread;
	init();
	welcome();
	initGame();
	printFrame();
	/*
	for (int i=0; i<7; i++) {
		for (int j=0; j<4; j++) {
			printBlock(8, 4, blocks[i][j], 1);
			getch();
			printBlock(8, 4, blocks[i][j], 0);
		}
	}
	//getch();
	*/
	pthread_create(&commThread, NULL, getCommand, NULL);
	pthread_create(&updateThread, NULL, updateGame, NULL);
	while (!gameOver);
	getch();
	endwin();
	return 0;
}

void init(){
	srand((int)time(0));
	initscr();
	cbreak();
	noecho();
	clear();
	curs_set(0);
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
	/*绘制游戏方框*/
	mvprintw(0, 0, "+");
	mvprintw(FRAME_H+1, 0, "+");
	mvprintw(0, FRAME_W+1, "+");
	mvprintw(FRAME_H+1, FRAME_W+1, "+");
	
	for (int i=1; i<=FRAME_W; i++) {
		//mvprintw(0, i, "-");
		mvprintw(FRAME_H+1, i, "-");
	}
	
	for (int i=1; i<=FRAME_H; i++) {
		mvprintw(i, 0, "|");
		mvprintw(i, FRAME_W+1, "|");
	}
	/*绘制下一个方块和得分*/
	mvprintw(1, FRAME_W + 4, "next");
	mvprintw(FRAME_H - 4, FRAME_W + 4, "score:");
	
	printBlock(4, FRAME_W + 4, blocks[nextBlock.id][nextBlock.shape], 1);
	return;
}

void initGame() {
	/*初始化方块数据*/
	setBlocks();
	/*初始化当前方块和下一个方块*/
	int next, shape;
	next = rand() % 7;
	shape = rand() % 4;
	curBlock.id = next;
	curBlock.shape = shape;
	next = rand() % 7;
	shape = rand() % 4;
	nextBlock.id = next;
	nextBlock.shape = shape;
	
	score = 0;//初始化得分
	curPosition.x = 0;
	curPosition.y = FRAME_W / 2;//初始化当前位置
	gameOver = 0;
	gamePause = 0;
	speed = 1;
	
	/*初始化游戏空间数组*/
	for (int i=0; i<=FRAME_H; i++) {
		for (int j=0; j<=FRAME_W; j++) {
			room[i][j] = 0;
		}
	}
	return ;
}

void newBlock() {//当前方块无法下落，结束当前方块，新生成一个方块
	int next = rand() % 7;
	int shape = rand() % 4;
	
	printBlock(4, FRAME_W + 4, blocks[nextBlock.id][nextBlock.shape], 0);
	curBlock = nextBlock;
	nextBlock.id = next;
	nextBlock.shape = shape;
	printBlock(4, FRAME_W + 4, blocks[nextBlock.id][nextBlock.shape], 1);
	refresh();
	curPosition.x = 0;
	curPosition.y = FRAME_W / 2;
	return ;
}

struct point makePoint(int x, int y) {
	struct point re;
	re.x = x;
	re.y = y;
	return re;
}

void printBlock(int x, int y, struct block target, int display){//在指定位置绘制方块
	struct point p;
	for (int i=0; i<4; i++) {
		p = target.p[i];
		if (display) {
			mvprintw(x + p.x, y + p.y, "#");
		}
		else {
			mvprintw(x + p.x, y + p.y, " ");
		}
		refresh();
	}
	
}

void *getCommand(void *args) {//监听控制命令的线程
	while (1) {
		int ch = getch();
		switch(ch) {
		case ROTATE:
			doRotate();
			break;
		case LEFT:
			doLeft();
			break;
		case RIGHT:
			doRight();
			break;
		case DOWN:
			if (ifBlockDrop()) 
				doBlockDrop();
			break;
		case PAUSE:
			doPause();
			break;
		case EXIT:
			doExit();
			break;
		}
	}
}

void doRotate() {//旋转操作的函数
	mvprintw(30, 0, "Rotate          ");
	int targetShape = (curBlock.shape + 1) % 4;
	struct point p;
	for (int i=0; i<4; i++) {//此循环判断当然是否可以旋转
		p = blocks[curBlock.id][targetShape].p[i];
		if (curPosition.x + p.x < 1 || curPosition.x + p.x > FRAME_H) {//判断旋转后是否越上下界
			return ;
		}
		if (curPosition.y + p.y < 1 || curPosition.y + p.y > FRAME_W) {//是否越左右界
			return;
		}
		if (room[curPosition.x + p.x][curPosition.y + p.y]) {//判断是否与已经放好的方块冲突
			return;
		}
	}
	//旋转操作
	printBlock(curPosition.x, curPosition.y, blocks[curBlock.id][curBlock.shape], 0);
	curBlock.shape = (curBlock.shape + 1) % 4;
	printBlock(curPosition.x, curPosition.y, blocks[curBlock.id][curBlock.shape], 1);
	refresh();
}

void doLeft() {
	struct point p;
        mvprintw(30, 0, "Left            ");
	for (int i=0; i<4; i++) {//此循环判断是否可以左移
		p = blocks[curBlock.id][curBlock.shape].p[i];
		if (curPosition.y + p.y <= 1) {//判断是否越界
			return;
		}
		if (room[curPosition.x + p.x][curPosition.y + p.y - 1]) {//判断是否被已放好的方块挡住
                        return;
                }
	} 
	//左移操作
	printBlock(curPosition.x, curPosition.y, blocks[curBlock.id][curBlock.shape], 0);
	curPosition.y --;
	printBlock(curPosition.x, curPosition.y, blocks[curBlock.id][curBlock.shape], 1);
	refresh();
}

void doRight() {//与左移类似
	struct point p;
        mvprintw(30, 0, "Right           ");
	for (int i=0; i<4; i++) {
                p = blocks[curBlock.id][curBlock.shape].p[i];
                if (curPosition.y + p.y >= FRAME_W) {
                        return;
                }
		if (room[curPosition.x + p.x][curPosition.y + p.y + 1]) {
			return;
		}
        }
	printBlock(curPosition.x, curPosition.y, blocks[curBlock.id][curBlock.shape], 0);
	curPosition.y++;
	printBlock(curPosition.x, curPosition.y, blocks[curBlock.id][curBlock.shape], 1);
	refresh();
}

void doBlockDrop() {//方块下落操作，在调用之前判断是否可以下落
	printBlock(curPosition.x, curPosition.y, blocks[curBlock.id][curBlock.shape], 0);
        curPosition.x ++;
        printBlock(curPosition.x, curPosition.y, blocks[curBlock.id][curBlock.shape], 1);
        refresh();
}

void doPause() {//暂停游戏
        mvprintw(30, 0, "Pause           ");
	if (gamePause) {
		gamePause = 0;
	}
	else {
		gamePause = 1;
	}
}

void doExit() {//退出游戏
        mvprintw(30, 0, "Exit            ");
	gameOver = 1;
}

void *updateGame(void *args){//定时更新当前方块的子线程的函数，主要是定时判断当前方块是否可以下落，是则下落，否则新生成方块
	struct point p;
	while (!gameOver) {
		while (gamePause);
		if (ifBlockDrop()) {
			doBlockDrop();
		}
		else {
			for (int i=0; i<4; i++) {//如果无法下落则设置当前方块占的游戏空间为1
				p = blocks[curBlock.id][curBlock.shape].p[i];
				room[curPosition.x + p.x][curPosition.y + p.y] = 1;
			}
			newBlock();
		}
		sleep(speed);//根据游戏速度睡眠
	}
}

int ifBlockDrop() {//判断当前方块是否可以下落
	struct point p;
	for (int i=0; i<4; i++) {//对于当前方块的每个小正方形判断
		p = blocks[curBlock.id][curBlock.shape].p[i];
		if (curPosition.x + p.x >= FRAME_H) {//判断是否到底
			return 0;
		}
		if (room[curPosition.x + p.x + 1][curPosition.y + p.y]) {//判断是否被已有方块挡住
			return 0;
		}
	}
	return 1;
}

void setBlocks() {
	/*设置每种方块每种形状的参数
	第一维下标0到6表示7种方块
	第二维下标0到3表示每种方块旋转后的形状
	p[0]到p[3]表示方块的四个小正方形的相对坐标位置，这里是相对p[0]的位置
	*/
	blocks[0][0].p[0] = makePoint(0, 0); blocks[0][0].p[1] = makePoint(1, 0); blocks[0][0].p[2] = makePoint(2, 0); blocks[0][0].p[3] = makePoint(-1, 0);
	blocks[0][1].p[0] = makePoint(0, 0); blocks[0][1].p[1] = makePoint(0, 1); blocks[0][1].p[2] = makePoint(0, 2); blocks[0][1].p[3] = makePoint(0, -1);
	blocks[0][2] = blocks[0][0];
	blocks[0][3] = blocks[0][1];

	blocks[1][0].p[0] = makePoint(0, 0); blocks[1][0].p[1] = makePoint(0, 1); blocks[1][0].p[2] = makePoint(1, 0); blocks[1][0].p[3] = makePoint(1, 1);
	blocks[1][1] = blocks[1][0];
	blocks[1][2] = blocks[1][0];
	blocks[1][3] = blocks[1][0];

	blocks[2][0].p[0] = makePoint(0, 0); blocks[2][0].p[1] = makePoint(-1, 0); blocks[2][0].p[2] = makePoint(1, 0); blocks[2][0].p[3] = makePoint(1, 1);
	blocks[2][1].p[0] = makePoint(0, 0); blocks[2][1].p[1] = makePoint(0, -1); blocks[2][1].p[2] = makePoint(0, 1); blocks[2][1].p[3] = makePoint(1, -1);
	blocks[2][2].p[0] = makePoint(0, 0); blocks[2][2].p[1] = makePoint(-1, 0); blocks[2][2].p[2] = makePoint(-1, -1);blocks[2][2].p[3]= makePoint(1, 0);
	blocks[2][3].p[0] = makePoint(0, 0); blocks[2][3].p[1] = makePoint(0, -1); blocks[2][3].p[2] = makePoint(0, 1); blocks[2][3].p[3] = makePoint(-1, 1);

	blocks[3][0].p[0] = makePoint(0, 0); blocks[3][0].p[1] = makePoint(-1, 0); blocks[3][0].p[2] = makePoint(1, 0); blocks[3][0].p[3] = makePoint(1, -1);
	blocks[3][1].p[0] = makePoint(0, 0); blocks[3][1].p[1] = makePoint(0, -1); blocks[3][1].p[2] = makePoint(0, 1); blocks[3][1].p[3] = makePoint(-1, -1);
	blocks[3][2].p[0] = makePoint(0, 0); blocks[3][2].p[1] = makePoint(-1, 0); blocks[3][2].p[2] = makePoint(-1, 1);blocks[3][2].p[3]= makePoint(1, 0);
	blocks[3][3].p[0] = makePoint(0, 0); blocks[3][3].p[1] = makePoint(0, -1); blocks[3][3].p[2] = makePoint(0, 1); blocks[3][3].p[3] = makePoint(1, 1);

	blocks[4][0].p[0] = makePoint(0, 0); blocks[4][0].p[1] = makePoint(0, -1); blocks[4][0].p[2] = makePoint(0, 1); blocks[4][0].p[3] = makePoint(1, 0);
	blocks[4][1].p[0] = makePoint(0, 0); blocks[4][1].p[1] = makePoint(-1, 0); blocks[4][1].p[2] = makePoint(1, 0); blocks[4][1].p[3] = makePoint(0, -1);
	blocks[4][2].p[0] = makePoint(0, 0); blocks[4][2].p[1] = makePoint(0, -1); blocks[4][2].p[2] = makePoint(0, 1); blocks[4][2].p[3] = makePoint(-1, 0);
	blocks[4][3].p[0] = makePoint(0, 0); blocks[4][3].p[1] = makePoint(-1, 0); blocks[4][3].p[2] = makePoint(1, 0); blocks[4][3].p[3] = makePoint(0, 1);

	blocks[5][0].p[0] = makePoint(0, 0); blocks[5][0].p[1] = makePoint(0, -1); blocks[5][0].p[2] = makePoint(1, 0); blocks[5][0].p[3] = makePoint(1, 1);
	blocks[5][1].p[0] = makePoint(0, 0); blocks[5][1].p[1] = makePoint(-1, 0); blocks[5][1].p[2] = makePoint(0, -1); blocks[5][1].p[3] = makePoint(1, -1);
	blocks[5][2] = blocks[5][0];
	blocks[5][3] = blocks[5][1];

	blocks[6][0].p[0] = makePoint(0, 0); blocks[6][0].p[1] = makePoint(0, 1); blocks[6][0].p[2] = makePoint(1, 0); blocks[6][0].p[3] = makePoint(1, -1);
	blocks[6][1].p[0] = makePoint(0, 0); blocks[6][1].p[1] = makePoint(-1, 0); blocks[6][1].p[2] = makePoint(0, 1); blocks[6][1].p[3] = makePoint(1, 1);
	blocks[6][2] = blocks[6][0];
	blocks[6][3] = blocks[6][1];
}
