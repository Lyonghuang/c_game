#include <curses.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#define CELL 4 //每行每列格子的数量
#define CELL_SIZE 6  //绘图时每个格子的大小
#define RIGHT 100
#define LEFT 97
#define UP 119
#define DOWN 115

void init();//初始化窗口
void welcome();//欢迎界面
void init_game();//初始化游戏数据
void start_game();//游戏主循环函数
void end_game();//结束游戏
int get_new_number();//随机生成2或4
void update_game();//刷新游戏进度
void do_right(); //游戏操作的四个函数，对应上下左右四个操作
void do_left();
void do_up();
void do_down();
int game_over();//游戏结束处理

int number[CELL][CELL];//游戏数字数组
int score = 0;//得分

int main() {
	init();
	welcome();
	init_game();
	start_game();
	//getch();
	//end_game();

}

void init(){
	srand((int)time(0));//设置随机数种子
	initscr();//获取终端窗口信息
	cbreak();//禁用行缓冲
	noecho();//禁用输入回显
	clear();//清屏
}

void welcome(){
	mvaddstr(0, 0, "Welcome to 2048! Please press 's' to start game\n");
	int ch;
	while ((ch = getch()) != 's'){
	
	}
	return;
}

void init_game() {
	clear();
	/*初始化格子里的数据*/
	for (int i=0; i<CELL; i++) {
		for (int j=0; j<CELL; j++) {
			number[i][j] = 0;
		}
	}
	/*得分归零*/
	score = 0;
	
	/*初始随机三个位置数字初始化为2或4*/
	int x, y;
	x = random() % CELL;
	y = random() % CELL;
	
	int count = 3;
	while (count) {
		if (number[x][y]){
			continue;
		}
		number[x][y] = get_new_number();
		x = random() % CELL;
		y = random() % CELL;
		count --;
	}
	
	update_game();
}

void start_game(){
	int command;
	command = getch();
	/*游戏各种操作对应的处理方法*/
	/*按下e可结束游戏*/
	/*按其他按键不作任何事*/
	while (1) {
		int error = 0;
		int exit = 0;
		switch (command) {
			case 'e':
				exit = 1;
				break;
			case RIGHT:
				do_right();
				break;
			case LEFT:
				do_left();
				break;
			case UP:
				do_up();
				break;
			case DOWN:
				do_down();
				break;
			default:
				error = 1;
				break;
		}
		if (exit) break;
		
		update_game();
		//command = getch();
		if (error) {
			command = getch();
			continue;
		}
		
		/*判断游戏是否结束*/
		if (game_over()) {
                        break;
                }
		
		/*随机位置生成随机2或4*/
		int x = random() % CELL;
		int y = random() % CELL;
		while (number[x][y]) {
			x = random() % CELL;
			y = random() % CELL;
		}
		number[x][y] = get_new_number();
		update_game();
		command = getch();
	}
	end_game();
}

void update_game() {
	/*画游戏CELLxCELL格子*/
	clear();
	for (int i=0; i<=CELL*CELL_SIZE; i++) {
                for (int j=0; j<=CELL*CELL_SIZE; j++) {
                        if (i % CELL_SIZE == 0 || j % CELL_SIZE == 0) {
                                mvaddch(i, j, '*');
                                refresh();
                        }
                }
        }
	refresh();

	/*将数字填充到格子里*/
	for (int i=0; i<CELL; i++) {
		for (int j=0; j<CELL; j++) {
			if (!number[i][j]) {
				continue;
			}
			mvprintw(i * CELL_SIZE + CELL_SIZE/2, j * CELL_SIZE + 1, "%4d", number[i][j]);
			refresh();
		}
	}

	/*得分*/
	mvprintw(CELL*CELL_SIZE+6, 0, "Score: %d\n", score);

	/*键位控制说明*/
	mvprintw(CELL*CELL_SIZE+8, 0, "Press 'W' to wipe up.");
	mvprintw(CELL*CELL_SIZE+9, 0, "Press 'S' to wipe down.");
	mvprintw(CELL*CELL_SIZE+10, 0, "Press 'A' to wipe left.");
	mvprintw(CELL*CELL_SIZE+11, 0, "Press 'D' to wipe right.");
	mvprintw(CELL*CELL_SIZE+12, 0, "Press 'E' to stop game.");
	refresh();
}

void end_game(){
	/*游戏结束显示得分，可选择重新游戏或退出游戏*/
	clear();
	mvaddstr(0, 0, "GAME OVER!\n");
	mvprintw(2, 0, "Your score is %d\n", score);
	mvprintw(4, 0, "Press 'n' to start a new game, Press 'q' to exit.\n");
	refresh();
	int ch = getch();
	while (ch != 'n' && ch != 'q'){
		ch = getch();
	}
	if (ch == 'n') {
		init_game();
		start_game();
	}
	else if (ch == 'q') {
		clear();
		mvaddstr(0, 0, "Bye!\n");
		refresh();
		sleep(2);
		endwin();
	}
	//refresh();
	//sleep(2);
	//endwin();
}

int game_over() {
/*如果所有格子满了那么判断为游戏结束*/
	for (int i=0; i<CELL; i++) {
		for (int j=0; j<CELL; j++) {
			if (!number[i][j]) {
				return 0;
			}
		}
	}
	return 1;
}

void do_right() {
/*每行自右往左依次处理格子里的数字，对于每一个非零数字尽量将其往右移，如果遇到相等的数字还可以合并继续右移*/
/*后面三个处理函数类似*/
	for (int i=0; i<CELL; i++) {
		for (int j = CELL-1; j >= 0; j--) {
			if (!number[i][j]) { //为零直接跳过
				continue;
			}
			int k = j;
			while (k < CELL - 1) {
				if (!number[i][k+1]) {//如果当前数字右边为0,直接右移一位
					number[i][k+1] = number[i][k];
					number[i][k] = 0;
				}
				else if (number[i][k] == number[i][k+1]) {//如果当前数字和右边数字相等，则相加到右边位置，得分增加
					score += number[i][k];
					number[i][k+1] += number[i][k+1];
					number[i][k] = 0;
				}
				else if (number[i][k] != number[i][k+1]) {//如果当前数字与右边数字不相等，说明无法右移，结束
					break;
				}
				k++;
			}
		}		
	}
}

void do_left() {
	for (int i=0; i<CELL; i++) {
		for (int j=0; j<CELL; j++) {
			if (!number[i][j]) {
				continue;
			}
			int k = j;
			while (k) {
				if (!number[i][k-1]) {
					number[i][k-1] = number[i][k];
					number[i][k] = 0;
				}
				else if (number[i][k-1] == number[i][k]) {
					score += number[i][k];
					number[i][k-1] += number[i][k];
					number[i][k] = 0;
				}
				else if (number[i][k-1] != number[i][k]) {
					break;
				}
				k--;
			}
		}
	}
}

void do_up() {
	for (int i=0; i<CELL; i++) {
		for (int j=0; j<CELL; j++) {
			if (!number[j][i]) {
				continue;
			}
			int k = j;
			while(k) {
				if (!number[k-1][i]) {
					number[k-1][i] = number[k][i];
					number[k][i] = 0;
				}
				else if (number[k-1][i] == number[k][i]) {
					score += number[k][i];
					number[k-1][i] += number[k][i];
					number[k][i] = 0;
				}
				else if (number[k-1][i] != number[k][i]) {
					break;
				}
				k--;
			}
		}
	}
}

void do_down(){
	for (int i=0; i<CELL; i++) {
		for (int j=CELL-1; j >= 0;j--) {
			if (!number[j][i]){
				continue;
			}
			int k = j;
			while (k < CELL -1) {
				if (!number[k+1][i]) {
					number[k+1][i] = number[k][i];
					number[k][i] = 0;
				}
				else if (number[k+1][i] == number[k][i]) {
					score += number[k][i];
					number[k+1][i] += number[k][i];
					number[k][i] = 0;
				}
				else if (number[k+1][i] != number[k][i]) {
					break;
				}
				k++;
			}
		}
	}
}

int get_new_number() {
	/*随机生成一个数字，10%概率为4, 90%概率为2*/
	int p = random() % 10;
	int re;
	if (p) {
		re = 2;
	}
	else {
		re = 4;
	}
	
	return re;
}

