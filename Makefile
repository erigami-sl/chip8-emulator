CFLAGS = -Wall -Wextra -std=c99 -I include

all:
	gcc src/main.c src/chip8.c $(CFLAGS) -o chip8
