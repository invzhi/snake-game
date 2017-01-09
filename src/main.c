#include <unistd.h>
#include <curses.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "config.h"

WINDOW* gameSpace;
int gameSpaceRow;
int gameSpaceCol;

int score;
// int highestScore;
int delayTime;
int initLength;
chtype snakeChar;

Snake* snake;
Fruit* fruit;

int initTUI(void);
void initData(void);
void gameLoop(void);

Snake* getSnake(void);
Fruit* getFruit(void);
void drawSnake(void);
void drawFruit(void);
void newFruit(Fruit* fruit);

void keyboardHandler(void);
void pauseGame(void);

int isSurvival(void);
void snakeGrowth(void);

// void parseOption(int argc, char* argv[]) {
// 	int i;
// 	for (i = 1; i < argc; i++) {
// 		if (strcmp(argv[i], "-s") == 0) {

// 		} else if (strcmp(argv[i], "-l") == 0) {
// 		} else if (strcmp(argv[i], "-r") == 0) {
// 		} else if (strcmp(argv[i], "-h") == 0) {
// 		} else {
// 			printf("Snake: Unknown option: %s\nSee \'snake -h\'\n", argv[i]);
// 			break;
// 		}
// 	}

// }

int main(int argc, char* argv[]) {
	// parseOption(argc, argv);
	if (initTUI() == OK) {
		initData();
		gameLoop();
	}
	return EXIT_SUCCESS;
}

int initTUI(void) {
	int status = OK;
	initscr();
	noecho();
	curs_set(0);
	nodelay(stdscr, TRUE);
	gameSpaceRow = LINES - INFO_HEIGHT;
	gameSpaceCol = COLS & (~1);
	if (gameSpaceRow - 2 < 10 || gameSpaceCol / 2 - 1 < 10) {
		status = ERR;
		endwin();
		puts("Snake: Now your terminal is too small. Please enlarge it and restart the snake game.");
	} else {
		gameSpace = subwin(stdscr, gameSpaceRow, gameSpaceCol, INFO_HEIGHT, 0);
		mvprintw(0, 1, "Score: %4d", 0);
		box(gameSpace, 0, 0);
	}
	return status;
}

void initData(void) {
	srand((unsigned int) time(NULL));
	
	score = 0;
	initLength = 3;
	snakeChar = 65568;

	snake = getSnake();
	fruit = getFruit();

	delayTime = INIT_DELAY_TIME;
}

void gameLoop(void) {
	drawSnake();
	drawFruit();
	while (isSurvival()) {
		wrefresh(gameSpace);
		usleep(delayTime);
		keyboardHandler();
		snakeGrowth();
	}
	delwin(gameSpace);
	mvprintw(0, gameSpaceCol / 2 - 1, "END");
	refresh();
	nodelay(stdscr, FALSE);
	getch();
	endwin();
}

Snake* getSnake(void) {
	Snake* snake = (Snake*) malloc(sizeof(Snake));
	snake -> length = initLength;
	snake -> direction = INIT_SNAKE_DIRECTION;

	int i;
	SnakeNode *snakeNode, *preSnakeNode = NULL;
	for (i = 0; i < snake -> length; i++) {
		snakeNode = (SnakeNode*) malloc(sizeof(SnakeNode));

		if (i == 0) snake -> head = snakeNode;
		if (i == snake -> length - 1) snake -> tail = snakeNode;

		snakeNode -> y = gameSpaceRow / 2;
		snakeNode -> x = (gameSpaceCol / 2 - i * 2) | 1;
		snakeNode -> next = preSnakeNode;
		preSnakeNode = snakeNode;
	}
	return snake;
}

Fruit* getFruit(void) {
	Fruit* fruit = (Fruit*) malloc(sizeof(Fruit));
	do {
		fruit -> y = rand() % (gameSpaceRow - 2) + 1;
		fruit -> x = (rand() % (gameSpaceCol - 2)) | 1;
	} while (mvwinch(gameSpace, fruit -> y, fruit -> x) == snakeChar);
	return fruit;
}

void drawSnake(void) {
	SnakeNode* p = snake -> tail;
	wattron(gameSpace, A_STANDOUT);
	while (p) {
		mvwprintw(gameSpace, p -> y, p -> x, BLOCK);
		p = p -> next;
	}
	wattroff(gameSpace, A_STANDOUT);
}

void drawFruit(void) {
	wattron(gameSpace, A_STANDOUT | A_DIM);
	mvwprintw(gameSpace, fruit -> y, fruit -> x, BLOCK);
	wattroff(gameSpace, A_STANDOUT | A_DIM);
}

void newFruit(Fruit* fruit) {
	do {
		fruit -> y = rand() % (gameSpaceRow - 2) + 1;
		fruit -> x = (rand() % (gameSpaceCol - 2)) | 1;
	} while (mvwinch(gameSpace, fruit -> y, fruit -> x) == snakeChar);
}

void keyboardHandler(void) {
	switch(getch()) {
		case 'w':
			if (snake -> direction != DOWN)  snake -> direction = UP;
			break;
		case 's':
			if (snake -> direction != UP)    snake -> direction = DOWN;
			break;
		case 'a':
			if (snake -> direction != RIGHT) snake -> direction = LEFT;
			break;
		case 'd':
			if (snake -> direction != LEFT)  snake -> direction = RIGHT;
			break;
		case 'p':
			pauseGame();
			break;
		case 'q':
			
			break;
	}
}

void pauseGame(void) {
	int pauseWinRow = 5, pauseWinCol = 18;
	WINDOW* pauseWin = newwin(pauseWinRow, pauseWinCol, gameSpaceRow / 2 - pauseWinRow / 2, gameSpaceCol / 2 - pauseWinCol / 2);
	box(pauseWin, 0, 0);
	mvwprintw(pauseWin, 1, 2, "<p> to resume");
	mvwprintw(pauseWin, 2, 2, "<r> to restart");
	mvwprintw(pauseWin, 3, 2, "<q> to quit");
	wrefresh(pauseWin);
	nodelay(stdscr, FALSE);
	switch(getch()) {
		case 'p':
			nodelay(stdscr, TRUE);
			werase(pauseWin);
			wrefresh(pauseWin);
			delwin(pauseWin);
			break;
		case 'r':
			//
			break;
		case 'q':
			//
			break;
	}

}

int isSurvival(void) {
	SnakeNode* p = snake -> tail;
	while (p -> next) {
		if (p -> y == snake -> head -> y && p -> x == snake -> head -> x) break;
		p = p -> next;
	}
	return !(p -> next);
}

void snakeGrowth(void) {
	SnakeNode* newNode;
	if (snake -> head -> y == fruit -> y && snake -> head -> x == fruit -> x) {
		newNode = (SnakeNode*) malloc(sizeof(SnakeNode));
		snake -> length++;
		if (delayTime > MIN_DELAY_TIME) delayTime -= 1000;
		mvprintw(0, 8, "%4d", ++score);
		newFruit(fruit);
		drawFruit();
	} else {
		mvwprintw(gameSpace, snake -> tail -> y, snake -> tail -> x, BLOCK);
		newNode = snake -> tail;
		snake -> tail = snake -> tail -> next;
	}
	newNode -> y = snake -> head -> y;
	newNode -> x = snake -> head -> x;
	newNode -> next = NULL;
	snake -> head = snake -> head -> next = newNode;
	switch(snake -> direction) {
		case UP:
			newNode -> y -= 1;
			break;
		case DOWN:
			newNode -> y += 1;
			break;
		case LEFT:
			newNode -> x -= 2;
			break;
		case RIGHT:
			newNode -> x += 2;
			break;
	}
	if (snake -> head -> y == 0) snake -> head -> y = gameSpaceRow - 2;
	else if (snake -> head -> x == -1) snake -> head -> x = gameSpaceCol - 3;
	else if (snake -> head -> y == gameSpaceRow - 1) snake -> head -> y = 1;
	else if (snake -> head -> x == gameSpaceCol - 1) snake -> head -> x = 1;
	wattron(gameSpace, A_STANDOUT);
	mvwprintw(gameSpace, snake -> head -> y, snake -> head -> x, BLOCK);
	wattroff(gameSpace, A_STANDOUT);
}