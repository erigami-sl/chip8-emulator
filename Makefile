CFLAGS = -Wall -Wextra -std=c99 -I include
LDLIBS = -lSDL2

all:
	gcc src/main.c src/chip8.c $(CFLAGS) -o chip8 $(LDLIBS)
