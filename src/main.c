#include <unistd.h>
#include <curses.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define BLOCK "  "
#define INFO_HEIGHT 1
#define MIN_DELAY_TIME 10000
#define INIT_DELAY_TIME 150000//
#define INIT_SNAKE_DIRECTION RIGHT

typedef enum {
	UP,
	DOWN,
	LEFT,
	RIGHT,
} Direction;

typedef struct Node {
	int y;
	int x;
	struct Node* next;
} SnakeNode;

typedef struct {
	int length;
	SnakeNode* head;
	SnakeNode* tail;
	Direction direction;
} Snake;

typedef struct {
	int y;
	int x;
} Fruit;

WINDOW* gameSpace;
int gameSpaceRow;
int gameSpaceCol;

// int highestScore;
int delayTime;
int initLength = 3;
chtype snakeCh = 65568;

Snake* snake;
Fruit* fruit;

Snake* getSnake() {
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

Fruit* getFruit() {
	Fruit* fruit = (Fruit*) malloc(sizeof(Fruit));
	do {
		fruit -> y = rand() % (gameSpaceRow - 2) + 1;
		fruit -> x = (rand() % (gameSpaceCol - 2)) | 1;
	} while (mvwinch(gameSpace, fruit -> y, fruit -> x) == snakeCh);
	return fruit;
}

void newFruit(Fruit* fruit) {
	do {
		fruit -> y = rand() % (gameSpaceRow - 2) + 1;
		fruit -> x = (rand() % (gameSpaceCol - 2)) | 1;
	} while (mvwinch(gameSpace, fruit -> y, fruit -> x) == snakeCh);
}

void initTUI() {
	initscr();
	noecho();
	curs_set(0);
	nodelay(stdscr, TRUE);
	gameSpaceRow = LINES - INFO_HEIGHT;
	gameSpaceCol = COLS & (~1);
	gameSpace = subwin(stdscr, gameSpaceRow, gameSpaceCol, INFO_HEIGHT, 0);
	box(gameSpace, 0, 0);
}

void initData() {
	srand((unsigned int) time(NULL));
	snake = getSnake();
	fruit = getFruit();
	delayTime = INIT_DELAY_TIME;
}

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
void drawSnake() {
	SnakeNode* p = snake -> tail;
	wattron(gameSpace, A_STANDOUT);
	while (p) {
		mvwprintw(gameSpace, p -> y, p -> x, BLOCK);
		p = p -> next;
	}
	wattroff(gameSpace, A_STANDOUT);
}

void keyboardHandler() {
	int ch = getch();
	switch(ch) {
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
			
			break;
		case 'q':
			
			break;
	}
}

void drawFruit() {
	wattron(gameSpace, A_STANDOUT | A_DIM);
	mvwprintw(gameSpace, fruit -> y, fruit -> x, BLOCK);
	wattroff(gameSpace, A_STANDOUT | A_DIM);
}

void snakeGrowth() {
	SnakeNode* newNode;
	if (snake -> head -> y == fruit -> y && snake -> head -> x == fruit -> x) {
		newNode = (SnakeNode*) malloc(sizeof(SnakeNode));
		snake -> length++;
		newFruit(fruit);
		drawFruit();
	} else {
		mvwprintw(gameSpace, snake -> tail -> y, snake -> tail -> x, BLOCK);
		newNode = snake -> tail;
		snake -> tail = snake -> tail -> next;
	}
	newNode -> y = snake -> head -> y;
	newNode -> x = snake -> head -> x;
	snake -> head = snake -> head -> next = newNode;
	newNode -> next = NULL;
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
	wattron(gameSpace, A_STANDOUT);
	mvwprintw(gameSpace, snake -> head -> y, snake -> head -> x, "  ");
	wattroff(gameSpace, A_STANDOUT);
}

int isSurvival() {
	SnakeNode* p = snake -> tail;
	while (p -> next) {
		if (p -> y == snake -> head -> y && p -> x == snake -> head -> x) break;
		p = p -> next;
	}
	return !(p -> next);
}

void gameLoop() {
	drawSnake();
	drawFruit();
	int i;
	while (isSurvival()) {
		wrefresh(gameSpace);
		usleep(delayTime -= 5);
		// sleep(1);
		keyboardHandler();
		snakeGrowth();
	}
	//
	mvprintw(0, gameSpaceCol / 2 - 1, "END");
	refresh();
	// nodelay(stdscr, FALSE);
	getch();
	endwin();
}

int main(int argc, char* argv[]) {
	// parseOption(argc, argv);
	initTUI();
	initData();
	gameLoop();
	return EXIT_SUCCESS;
}