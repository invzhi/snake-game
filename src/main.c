#include <unistd.h>
#include <curses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "config.h"

WINDOW* gameSpace;
int gameSpaceRow;
int gameSpaceCol;

int delayTime;
int defaultDelayTime = DEFAULT_DELAY_TIME;
int defaultSnakeLength = DEFAULT_SNAKE_LENGTH;

int score;
// int highestScore;

chtype snakeChar;

Snake* snake;
Food* food;

void parseOption(int argc, char* argv[]);
int initTUI(void);
void initData(void);
void gameLoop(void);

WINDOW* displayPauseWin(int pauseGameWinRow, int pauseGameWinCol);
WINDOW* displayGameOverWin(int gameOverWinRow, int gameOverWinCol);

Snake* getSnake(void);
Food* getFood(void);
void drawFood(void);
void newFood(Food* food);

void keyboardHandler(void);
void pauseGame(void);
void displaySnakeHead(void);
void displayAchievement(void);

int isSurvival(void);
void snakeGrowth(void);

int main(int argc, char* argv[]) {
	parseOption(argc, argv);
	if (initTUI() == OK) {
		initData();
		gameLoop();
		displayAchievement();
	}
	return EXIT_SUCCESS;
}

void parseOption(int argc, char* argv[]) {
	char* endptr;
	int i;
	for (i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
			puts(HELP_MANUAL);
		} else if (strcmp(argv[i], "-s") == 0 || strcmp(argv[i], "--speed") == 0) {
			if (++i < argc) {
				int speed = (int) strtol(argv[i], &endptr, 10);
				if (*endptr == '\0' && speed >= 1 && speed <= 10) {
					defaultDelayTime = (MIN_DELAY_TIME - DEFAULT_DELAY_TIME) / 9 * speed + (10 * DEFAULT_DELAY_TIME - MIN_DELAY_TIME) / 9;
					continue;
				}
			}
			printf("fatal: \'%s\' is not a number in 1 - 10.\n", argv[i]);
		} else if (strcmp(argv[i], "-l") == 0 || strcmp(argv[i], "--length") == 0) {
			if (++i < argc) {
				int length = (int) strtol(argv[i], &endptr, 10);
				if (*endptr == '\0' && length >= 1 && length <= 100) {
					defaultSnakeLength = length;
					continue;
				}
			}
			printf("fatal: \'%s\' is not a number in 2 - 100.\n", argv[i]);
		// } else if (strcmp(argv[i], "-r") == 0 || strcmp(argv[i], "--reset") == 0) {
		} else {
			printf("snake: \'%s\' is not a snake option. See \'snake --help\'.\n", argv[i]);
		}
		exit(EXIT_SUCCESS);
	}
}

int initTUI(void) {
	int status = OK;
	initscr();
	noecho();
	curs_set(0);
	nodelay(stdscr, TRUE);
	gameSpaceRow = LINES - INFO_WINDOW_HEIGHT;
	gameSpaceCol = COLS & (~1);
	if (gameSpaceRow - 2 < 10 || gameSpaceCol / 2 - 1 < 10) {
		status = ERR;
		endwin();
		puts("snake: Your window is smaller than 13 * 22.\nPlease enlarge it and try again.");
	} else {
		gameSpace = subwin(stdscr, gameSpaceRow, gameSpaceCol, INFO_WINDOW_HEIGHT, 0);
		mvprintw(0, 1, "SCORE: 0");
		box(gameSpace, 0, 0);
	}
	return status;
}

void initData(void) {
	srand((unsigned int) time(NULL));
	score = 0;
	snakeChar = 65568;
	delayTime = defaultDelayTime;
	snake = getSnake();
	food = getFood();
}

void gameLoop(void) {
	drawFood();
	while (isSurvival()) {
		snakeGrowth();
		wrefresh(gameSpace);
		usleep(delayTime);
		keyboardHandler();
	}
	displaySnakeHead();
	sleep(1);
	displayGameOverWin(3, 13);
	nodelay(stdscr, FALSE);
	getch();
	endwin();
}

WINDOW* displayPauseWin(int pauseGameWinRow, int pauseGameWinCol) {
	WINDOW* pauseGameWin = newwin(pauseGameWinRow, pauseGameWinCol, gameSpaceRow / 2 - pauseGameWinRow / 2, gameSpaceCol / 2 - pauseGameWinCol / 2);
	box(pauseGameWin, 0, 0);
	mvwprintw(pauseGameWin, 1, 2, "<p> to resume");
	// mvwprintw(pauseGameWin, 2, 2, "<r> to restart");
	mvwprintw(pauseGameWin, 3, 2, "<q> to quit");
	wrefresh(pauseGameWin);
	return pauseGameWin;
}

WINDOW* displayGameOverWin(int gameOverWinRow, int gameOverWinCol) {
	WINDOW* gameOverWin = subwin(stdscr, gameOverWinRow, gameOverWinCol, gameSpaceRow / 2 - gameOverWinRow / 2, gameSpaceCol / 2 - gameOverWinCol / 2);
	box(gameOverWin, 0, 0);
	mvwprintw(gameOverWin, 1, 1, " GAME OVER ");
	wrefresh(gameOverWin);
	return gameOverWin;
}

Snake* getSnake(void) {
	Snake* snake = (Snake*) malloc(sizeof(Snake));
	snake -> length = defaultSnakeLength;
	snake -> direction = DEFAULT_SNAKE_DIRECTION;
	int i;
	SnakeNode *snakeNode, *preSnakeNode = NULL;
	for (i = 0; i < snake -> length; i++) {
		snakeNode = (SnakeNode*) malloc(sizeof(SnakeNode));
		if (i == 0)                   snake -> head = snakeNode;
		if (i == snake -> length - 1) snake -> tail = snakeNode;
		snakeNode -> y = gameSpaceRow / 2;
		snakeNode -> x = -1 - 2 * i;
		snakeNode -> next = preSnakeNode;
		preSnakeNode = snakeNode;
	}
	return snake;
}

Food* getFood(void) {
	Food* food = (Food*) malloc(sizeof(Food));
	do {
		food -> y = rand() % (gameSpaceRow - 2) + 1;
		food -> x = (rand() % (gameSpaceCol - 2)) | 1;
	} while (mvwinch(gameSpace, food -> y, food -> x) == snakeChar);
	return food;
}

void drawFood(void) {
	wattron(gameSpace, A_STANDOUT | A_DIM);
	mvwprintw(gameSpace, food -> y, food -> x, BLOCK);
	wattroff(gameSpace, A_STANDOUT | A_DIM);
}

void newFood(Food* food) {
	do {
		food -> y = rand() % (gameSpaceRow - 2) + 1;
		food -> x = (rand() % (gameSpaceCol - 2)) | 1;
	} while (mvwinch(gameSpace, food -> y, food -> x) == snakeChar);
}

void keyboardHandler(void) {
	static Direction direction;
	do {
		switch(getch()) {
			case 'w':
				direction = UP;
				break;
			case 's':
				direction = DOWN;
				break;
			case 'a':
				direction = LEFT;
				break;
			case 'd':
				direction = RIGHT;
				break;
			case ERR:
				return;
			case 'p':
				pauseGame();
			default:
				direction = snake -> direction;
		}
	} while ((snake -> direction & 2) == (direction & 2));
	snake -> direction = direction;
}

void pauseGame(void) {
	int isIllegal;
	WINDOW* pauseGameWin = displayPauseWin(5, 18);
	nodelay(stdscr, FALSE);
	do {
		isIllegal = 0;
		switch(getch()) {
			case 'p':
				nodelay(stdscr, TRUE);
				mvwprintw(pauseGameWin, 1, 2, " 1s");
				wrefresh(pauseGameWin);
				sleep(1);
				delwin(pauseGameWin);
				touchwin(stdscr);
				wrefresh(gameSpace);
				break;
			// case 'r':
				
			// 	break;
			case 'q':
				endwin();
				displayAchievement();
				exit(EXIT_SUCCESS);
			default:
				isIllegal = 1;
				break;
		}
	} while (isIllegal);
}

void displaySnakeHead(void) {
	wattron(gameSpace, A_STANDOUT | A_DIM);
	mvwprintw(gameSpace, snake -> head -> y, snake -> head -> x, BLOCK);
	wattroff(gameSpace, A_STANDOUT | A_DIM);
	wrefresh(gameSpace);
}

void displayAchievement(void) {
	printf("Snake's length is %d.\n", snake -> length);
	printf("Your score is %d.\n", score);
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
	static SnakeNode* newSnakeNode;
	if (snake -> head -> y == food -> y && snake -> head -> x == food -> x) {
		newSnakeNode = (SnakeNode*) malloc(sizeof(SnakeNode));
		if (delayTime > MIN_DELAY_TIME) delayTime -= REDUCED_DELAY_TIME;
		snake -> length++;
		mvprintw(0, 8, "%d", ++score);
		newFood(food);
		drawFood();
	} else {
		mvwprintw(gameSpace, snake -> tail -> y, snake -> tail -> x, BLOCK);
		newSnakeNode = snake -> tail;
		snake -> tail = snake -> tail -> next;
	}
	newSnakeNode -> y = snake -> head -> y;
	newSnakeNode -> x = snake -> head -> x;
	newSnakeNode -> next = NULL;
	snake -> head = snake -> head -> next = newSnakeNode;
	switch(snake -> direction) {
		case UP:
			newSnakeNode -> y -= 1;
			break;
		case DOWN:
			newSnakeNode -> y += 1;
			break;
		case LEFT:
			newSnakeNode -> x -= 2;
			break;
		case RIGHT:
			newSnakeNode -> x += 2;
			break;
	}
	if      (snake -> head -> y == 0)  snake -> head -> y = gameSpaceRow - 2;
	else if (snake -> head -> x == -1) snake -> head -> x = gameSpaceCol - 3;
	else if (snake -> head -> y == gameSpaceRow - 1) snake -> head -> y = 1;
	else if (snake -> head -> x == gameSpaceCol - 1) snake -> head -> x = 1;
	wattron(gameSpace, A_STANDOUT);
	mvwprintw(gameSpace, snake -> head -> y, snake -> head -> x, BLOCK);
	wattroff(gameSpace, A_STANDOUT);
}