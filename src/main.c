#include <unistd.h>
#include <curses.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "config.h"

WINDOW* gameSpace;
int gameSpaceRow;
int gameSpaceCol;

int delayTime;
int defaultDelayTime = DEFAULT_DELAT_TIME;
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

Snake* getSnake(void);
Food* getFood(void);
void drawSnake(void);
void drawFood(void);
void newFood(Food* food);

void keyboardHandler(void);
void pauseGame(void);

int isSurvival(void);
void snakeGrowth(void);

int main(int argc, char* argv[]) {
	parseOption(argc, argv);
	if (initTUI() == OK) {
		initData();
		gameLoop();
	}
	return EXIT_SUCCESS;
}

void parseOption(int argc, char* argv[]) {
	char* endptr;
	int i;
	for (i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
			puts(HELP_MANUAL);
			exit(EXIT_SUCCESS);
		} else if (strcmp(argv[i], "-s") == 0 || strcmp(argv[i], "--speed") == 0) {
			if (++i < argc) {
				int speed = (int) strtol(argv[i], &endptr, 10);
				if (*endptr == '\0' && speed >= 1 && speed <= 10) {
					defaultDelayTime = (11 - speed) * 20000;
					continue;
				}
			}
			printf("fatal: \'%s\' is not a number in 1 - 10.\n", argv[i]);
			exit(EXIT_SUCCESS);
		} else if (strcmp(argv[i], "-l") == 0 || strcmp(argv[i], "--length") == 0) {
			if (++i < argc) {
				int length = (int) strtol(argv[i], &endptr, 10);
				if (*endptr == '\0' && length >= 1 && length <= 100) {
					defaultSnakeLength = length;
					continue;
				}
			}
			printf("fatal: \'%s\' is not a number in 2 - 100.\n", argv[i]);
			exit(EXIT_SUCCESS);
		// } else if (strcmp(argv[i], "-r") == 0 || strcmp(argv[i], "--reset") == 0) {
		} else {
			printf("snake: \'%s\' is not a snake option. See \'snake --help\'.\n", argv[i]);
			break;
		}
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
		puts("snake: Your window is smaller than 12 * 24.\nPlease enlarge it and try again.");
	} else {
		gameSpace = subwin(stdscr, gameSpaceRow, gameSpaceCol, INFO_WINDOW_HEIGHT, 0);
		mvprintw(0, 1, "Score: %*d", 4, 0);
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
	drawSnake();
	drawFood();
	while (isSurvival()) {
		wrefresh(gameSpace);
		usleep(delayTime);
		keyboardHandler();
		snakeGrowth();
	}
	mvprintw(0, gameSpaceCol / 2 - 4, "GAME OVER");
	refresh();
	nodelay(stdscr, FALSE);
	getch();
	delwin(gameSpace);
	endwin();
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
		snakeNode -> x = 1 - 2 * i;
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

void drawSnake(void) {
	SnakeNode* p = snake -> tail;
	wattron(gameSpace, A_STANDOUT);
	while (p) {
		mvwprintw(gameSpace, p -> y, p -> x, BLOCK);
		p = p -> next;
	}
	wattroff(gameSpace, A_STANDOUT);
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
	}
}

void pauseGame(void) {
	int status;
	int pauseWinRow = 5, pauseWinCol = 18;
	WINDOW* pauseWin = newwin(pauseWinRow, pauseWinCol, gameSpaceRow / 2 - pauseWinRow / 2, gameSpaceCol / 2 - pauseWinCol / 2);
	box(pauseWin, 0, 0);
	mvwprintw(pauseWin, 1, 2, "<p> to resume");
	// mvwprintw(pauseWin, 2, 2, "<r> to restart");
	mvwprintw(pauseWin, 3, 2, "<q> to quit");
	wrefresh(pauseWin);
	nodelay(stdscr, FALSE);
	do {
		status = 0;
		switch(getch()) {
			case 'p':
				nodelay(stdscr, TRUE);
				delwin(pauseWin);
				touchwin(stdscr);
				wrefresh(gameSpace);
				break;
			// case 'r':
				
			// 	break;
			case 'q':
				exit(EXIT_SUCCESS);
				break;
			default:
				status = 1;
				break;
		}
	} while (status);
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
	SnakeNode* newSnakeNode;
	if (snake -> head -> y == food -> y && snake -> head -> x == food -> x) {
		newSnakeNode = (SnakeNode*) malloc(sizeof(SnakeNode));
		if (delayTime > MIN_DELAY_TIME) delayTime -= REDUCED_DELAY_TIME;
		snake -> length++;
		mvprintw(0, 8, "%*d", 4, ++score);
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