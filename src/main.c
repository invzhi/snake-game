#include <unistd.h>
#include <getopt.h>
#include <ncurses.h>
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

void drawBlock(int attrs, int y, int x);
WINDOW* displayPauseWin(int pauseGameWinRow, int pauseGameWinCol);
WINDOW* displayGameOverWin(int gameOverWinRow, int gameOverWinCol);

Snake* getSnake(void);
Food* getFood(void);
void newFood(Food* food);

void keyboardHandler(void);
void pauseGame(void);
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
	char opt, *endptr;
	int speed, length;
	while ((opt = getopt_long(argc, argv, ":hs:l:", longopts, NULL)) != -1) {
		switch (opt) {
			case 'h':
				puts(HELP_MANUAL);
				exit(EXIT_SUCCESS);
				break;
			case 's':
				speed = (int) strtol(optarg, &endptr, 10);
				if (*endptr == '\0' && speed >= 1 && speed <= 10) {
					defaultDelayTime = (MIN_DELAY_TIME - DEFAULT_DELAY_TIME) / 9 * speed + (10 * DEFAULT_DELAY_TIME - MIN_DELAY_TIME) / 9;
					break;
				} else {
					printf("fatal: \'%s\' is not a value in 1 - 10.\n", optarg);
					exit(EXIT_SUCCESS);
				}
			case 'l':
				length = (int) strtol(optarg, &endptr, 10);
				if (*endptr == '\0' && length >= 1 && length <= 100) {
					defaultSnakeLength = length;
					break;
				} else {
					printf("fatal: \'%s\' is not a value in 2 - 100.\n", optarg);
					exit(EXIT_SUCCESS);
				}
			case ':':
				printf("fatal: option \'%c\' need a value.\n", optopt);
				exit(EXIT_SUCCESS);
				break;
			case '?':
				printf("snake: \'%s\' is not a snake option. See \'snake --help\'.\n", argv[optind]);
				exit(EXIT_SUCCESS);
				break;
		}
	}
	if (optind < argc) {
		printf("snake: \'%s\' is not a snake option. See \'snake --help\'.\n", argv[optind]);
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
	snakeChar = ' ' | SNAKE_ATTRIBUTES;
	delayTime = defaultDelayTime;
	snake = getSnake();
	food = getFood();
}

void gameLoop(void) {
	drawBlock(FOOD_ATTRIBUTES, food -> y, food -> x);
	while (isSurvival()) {
		snakeGrowth();
		wrefresh(gameSpace);
		usleep(delayTime);
		keyboardHandler();
	}
	drawBlock(IMPACT_ATTRIBUTES, snake -> head -> y, snake -> head -> x);
	wrefresh(gameSpace);
	sleep(1);
	displayGameOverWin(3, 13);
	nodelay(stdscr, FALSE);
	getch();
	endwin();
}

void drawBlock(int attrs, int y, int x) {
	wattrset(gameSpace, attrs);
	mvwprintw(gameSpace, y, x, BLOCK);
}

WINDOW* displayPauseWin(int pauseGameWinRow, int pauseGameWinCol) {
	WINDOW* pauseGameWin = newwin(pauseGameWinRow, pauseGameWinCol, (gameSpaceRow - pauseGameWinRow) / 2, (gameSpaceCol - pauseGameWinCol) / 2);
	box(pauseGameWin, 0, 0);
	mvwprintw(pauseGameWin, 1, 2, "<p> to resume");
	// mvwprintw(pauseGameWin, 2, 2, "<r> to restart");
	mvwprintw(pauseGameWin, 3, 2, "<q> to quit");
	wrefresh(pauseGameWin);
	return pauseGameWin;
}

WINDOW* displayGameOverWin(int gameOverWinRow, int gameOverWinCol) {
	WINDOW* gameOverWin = newwin(gameOverWinRow, gameOverWinCol, (gameSpaceRow - gameOverWinRow) / 2, (gameSpaceCol - gameOverWinCol) / 2);
	box(gameOverWin, 0, 0);
	mvwprintw(gameOverWin, 1, 2, "GAME OVER");
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

void displayAchievement(void) {
	printf("LENGTH:%5d\n", snake -> length);
	printf("SCORE: %5d\n", score);
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
		drawBlock(FOOD_ATTRIBUTES, food -> y, food -> x);
	} else {
		drawBlock(EMPTY_ATTRIBUTES, snake -> tail -> y, snake -> tail -> x);
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
	drawBlock(SNAKE_ATTRIBUTES, snake -> head -> y, snake -> head -> x);
}