CC = gcc

all: snake

snake: src/main.c src/config.h
	$(CC) src/main.c -o snake -lcurses

clean:
	rm snake