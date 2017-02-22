CC = gcc

all: snake

snake: src/main.c src/config.h
	$(CC) src/main.c -o snake -lncurses

.PHONY: clean
clean:
	-$(RM) snake