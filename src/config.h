#ifndef __CONFIG_H__
#define __CONFIG_H__

#define INFO_WINDOW_HEIGHT      1

#define BLOCK                   "  "
#define EMPTY_ATTRIBUTES        A_NORMAL
#define SNAKE_ATTRIBUTES        A_STANDOUT
#define FOOD_ATTRIBUTES         A_STANDOUT | A_DIM
#define IMPACT_ATTRIBUTES       A_STANDOUT | A_DIM

#define MIN_DELAY_TIME          20000
#define DEFAULT_DELAY_TIME      200000
#define REDUCED_DELAY_TIME      1000

#define DEFAULT_SNAKE_LENGTH    3
#define DEFAULT_SNAKE_DIRECTION RIGHT

#define HELP_MANUAL                                      \
	"SNAKE - the snake game on terminal.\n"              \
	"\n"                                                 \
	"SYNOPSIS\n"                                         \
	"\tsnake [OPTIONS]...\n"                             \
	"\n"                                                 \
	"OPTIONS\n"                                          \
	"\t-h, --help\n"                                     \
	"\t\tSee help.\n"                                    \
	"\n"                                                 \
	"\t-s number, --speed number\n"                      \
	"\t\tUse the number (1 - 10) as the game speed.\n"   \
	"\n"                                                 \
	"\t-l number, --length number\n"                     \
	"\t\tUse the number (2 - 100) as the snake length."  \

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
} Food;

#endif