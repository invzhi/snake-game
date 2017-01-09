#ifndef __CONFIG_H__
#define __CONFIG_H__

#define BLOCK "  "
#define INFO_HEIGHT 1
#define MIN_DELAY_TIME 20000
#define INIT_DELAY_TIME 200000//
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

#endif